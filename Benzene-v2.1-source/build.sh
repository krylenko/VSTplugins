#!/usr/bin/env bash
# ============================================================
#  Benzene — VST3 cross-compile build script
#
#  Builds the Windows x86-64 VST3 from Linux using clang-cl 18,
#  the xwin-fetched Windows SDK, JUCE 8, Ninja, and Wine.
#
#  Run SETUP.sh ONCE first to install the toolchain + Windows SDK.
#  Then run this script to (re)build. It is safe to re-run; it
#  re-applies all the per-configure Ninja patches automatically.
#
#  Usage:
#     ./build.sh                 # configure (if needed) + build VST3
#     ./build.sh clean           # wipe the build dir and rebuild from scratch
#
#  Environment overrides:
#     JUCE_DIR     path to JUCE 8 tree        (default: $HOME/JUCE)
#     XWIN         path to xwin SDK splat     (default: /opt/xwin-sdk)
#     BUILD_DIR    build output directory     (default: <project>/build-win)
# ============================================================
set -euo pipefail

# --- resolve paths ---
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JUCE_DIR="${JUCE_DIR:-$HOME/JUCE}"
XWIN="${XWIN:-/opt/xwin-sdk}"
SDKVER="10.0.26100"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build-win}"
TOOLCHAIN="$PROJECT_DIR/clang-cl-toolchain.cmake"

# Plugin/product naming — keep in sync with CMakeLists.txt
PRODUCT="Benzene"
TARGET="${PRODUCT}_VST3"

echo ">> Benzene build"
echo "   project:   $PROJECT_DIR"
echo "   JUCE:      $JUCE_DIR"
echo "   xwin SDK:  $XWIN"
echo "   build dir: $BUILD_DIR"

# --- sanity checks ---
[ -d "$JUCE_DIR/modules" ] || { echo "!! JUCE not found at $JUCE_DIR (set JUCE_DIR)"; exit 1; }
[ -d "$XWIN/sdk/include/$SDKVER" ] || { echo "!! Windows SDK not found at $XWIN — run SETUP.sh first"; exit 1; }
command -v wine >/dev/null || { echo "!! wine not found (needed for VST3 manifest)"; exit 1; }

if [ "${1:-}" = "clean" ]; then
    echo ">> clean: removing $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# --- configure ---
echo ">> configuring (CMake + Ninja)"
cmake "$PROJECT_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_BUILD_TYPE=Release \
    -DJUCE_DIR="$JUCE_DIR"

# ============================================================
#  Per-configure Ninja patches (MUST be re-applied after every
#  cmake run; CMake/Ninja regenerate these files each configure).
# ============================================================

# 1. rules.ninja: clang-cl chokes on the "-c -- $in" form CMake emits.
echo ">> patch: rules.ninja ( -c -- \$in  ->  -c \$in )"
sed -i 's/ -c -- \$in/ -c \$in/g' CMakeFiles/rules.ninja

# 2-4. build.ninja patches, applied together in Python for clarity.
echo ">> patch: build.ninja (vst3_helper toolchain, RC includes, Wine wrapper)"
HELPER_SH="$PROJECT_DIR/run_vst3_helper.sh"
python3 - "$TOOLCHAIN" "$XWIN" "$SDKVER" "$HELPER_SH" "$PRODUCT" << 'PYEOF'
import sys
toolchain, xwin, sdkver, helper_sh, product = sys.argv[1:6]
with open('build.ninja') as f:
    c = f.read()
n = 0

# (A) JUCE's nested vst3_helper CMake doesn't inherit the parent toolchain.
#     Forward the cross-compile settings to that sub-cmake invocation.
old = '-DCMAKE_RC_COMPILER=/usr/local/bin/llvm-rc && /usr/bin/cmake --build'
new = ('-DCMAKE_RC_COMPILER=/usr/local/bin/llvm-rc '
       '-DCMAKE_LINKER=/usr/local/bin/lld-link '
       '-DCMAKE_MT=/usr/local/bin/llvm-mt '
       '-DCMAKE_BUILD_TYPE=Release '
       f'-DCMAKE_TOOLCHAIN_FILE={toolchain} && /usr/bin/cmake --build')
if old in c: c = c.replace(old, new); n += 1
else: print("   (A) vst3_helper toolchain pattern not found (may already be patched)")

# (B) The .rc resource compile needs the Windows SDK include paths.
old = f'  FLAGS = -DWIN32\n  OBJECT_DIR = CMakeFiles/{product}_rc_lib.dir'
new = (f'  FLAGS = -DWIN32\n'
       f'  INCLUDES = -I{xwin}/sdk/include/{sdkver}/um '
       f'-I{xwin}/sdk/include/{sdkver}/shared '
       f'-I{xwin}/crt/include '
       f'-I{xwin}/sdk/include/{sdkver}/ucrt\n'
       f'  OBJECT_DIR = CMakeFiles/{product}_rc_lib.dir')
if old in c: c = c.replace(old, new); n += 1
else: print("   (B) RC include pattern not found (may already be patched)")

# (C) vst3_helper.exe must run under Wine (and with the VST3 path as an arg).
#     Replace the bare invocation with our wrapper script.
old = (f'/root/benzene/build-win/{product}_artefacts/JuceLibraryCode/'
       f'vst3_helper/vst3_helper.exe > ')
# The hardcoded /root/benzene path above only matches the original author's tree;
# match generically instead:
import re
pat = re.compile(
    r'(\S*)' + re.escape(product) +
    r'_artefacts/JuceLibraryCode/vst3_helper/vst3_helper\.exe > ')
c2, k = pat.subn(helper_sh + ' > ', c)
if k: c = c2; n += 1
else: print("   (C) vst3_helper invocation pattern not found (may already be patched)")

with open('build.ninja', 'w') as f:
    f.write(c)
print(f"   build.ninja patches applied: {n}")
PYEOF

# Generate the Wine wrapper the build.ninja patch points at. JUCE invokes the
# helper via a path that gets the build dir prepended, so a standalone script
# (which cd's itself) is the robust way to run it under Wine.
cat > "$HELPER_SH" << SH
#!/usr/bin/env bash
# Auto-generated by build.sh — runs JUCE's vst3_helper under Wine to emit
# moduleinfo.json. Wine needs no display; suppress its debug chatter.
cd "$BUILD_DIR"
exec env DISPLAY= WINEDEBUG=-all wine \\
  ${PRODUCT}_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe \\
  ${PRODUCT}_artefacts/Release/VST3/${PRODUCT}.vst3/Contents/x86_64-win/${PRODUCT}.vst3
SH
chmod +x "$HELPER_SH"

# --- build ---
echo ">> building $TARGET"
ninja "$TARGET"

VST3="$BUILD_DIR/${PRODUCT}_artefacts/Release/VST3/${PRODUCT}.vst3"
echo ""
echo ">> done."
echo "   VST3 bundle: $VST3"
echo "   Install by copying the ${PRODUCT}.vst3 folder to your Windows VST3 dir,"
echo "   e.g.  C:\\Program Files\\Common Files\\VST3\\"
