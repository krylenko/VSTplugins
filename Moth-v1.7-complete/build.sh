#!/bin/bash
# build.sh — configure, patch, and build Moth VST3 for Windows.
# Reapplies all ninja patches automatically (needed after every cmake run).
# Usage:  ./build.sh            (incremental build)
#         ./build.sh reconfigure (force a fresh cmake configure first)
set -e

ROOT=/home/claude/Moth
BUILD=$ROOT/build-win
TOOLCHAIN=$ROOT/clang-cl-toolchain.cmake
SDK=/opt/xwin-sdk/sdk/include/10.0.26100
CRT=/opt/xwin-sdk/crt/include
JOBS=$(nproc); [ "$JOBS" -lt 4 ] && JOBS=4   # JUCE TUs are independent; oversubscribe

source "$HOME/.cargo/env" 2>/dev/null || true

mkdir -p "$BUILD"; cd "$BUILD"

NEED_CONFIGURE=0
[ "$1" = "reconfigure" ] && NEED_CONFIGURE=1
[ ! -f build.ninja ] && NEED_CONFIGURE=1

if [ "$NEED_CONFIGURE" = "1" ]; then
    echo "== configuring =="
    cmake .. -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DCMAKE_BUILD_TYPE=Release > /tmp/moth_cmake.log 2>&1 \
        || { tail -20 /tmp/moth_cmake.log; exit 1; }

    echo "== patching rules.ninja =="
    sed -i 's/ -c -- \$in/ -c \$in/g' CMakeFiles/rules.ninja
    sed -i 's/ -E -- \$in/ -E \$in/g' CMakeFiles/rules.ninja

    echo "== patching build.ninja (VST3 helper toolchain, RC includes, wine) =="
    python3 - "$TOOLCHAIN" "$SDK" "$CRT" << 'PYEOF'
import sys
toolchain, sdk, crt = sys.argv[1], sys.argv[2], sys.argv[3]
with open('build.ninja') as f: c = f.read()
patches = [
  ('-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 && /usr/bin/cmake --build',
   f'-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 -DCMAKE_LINKER=/usr/local/bin/lld-link -DCMAKE_MT=/usr/local/bin/llvm-mt -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE={toolchain} && /usr/bin/cmake --build'),
  ('  FLAGS = -DWIN32\n  OBJECT_DIR = CMakeFiles/Moth_rc_lib.dir',
   f'  FLAGS = -DWIN32\n  INCLUDES = -I{sdk}/um -I{sdk}/shared -I{crt} -I{sdk}/ucrt\n  OBJECT_DIR = CMakeFiles/Moth_rc_lib.dir'),
  ('&& /home/claude/Moth/build-win/Moth_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe > ',
   '&& DISPLAY= WINEDEBUG=-all wine /home/claude/Moth/build-win/Moth_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe > '),
]
for old, new in patches:
    if old in c: c = c.replace(old, new)
    else: print("WARN: patch target not found:", old[:50], file=sys.stderr)
with open('build.ninja','w') as f: f.write(c)
print("patches applied")
PYEOF
fi

echo "== building (-j$JOBS) =="
ninja -j"$JOBS" Moth_VST3

echo "== done =="
file "$BUILD"/Moth_artefacts/Release/VST3/Moth.vst3/Contents/x86_64-win/Moth.vst3
