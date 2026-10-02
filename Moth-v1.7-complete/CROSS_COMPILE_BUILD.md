# Dragonfly FX — Cross-Compilation Build Guide

Cross-compiles a Windows x86-64 VST3 plugin from Linux (Ubuntu 24.04)
using clang-cl 18, the xwin-fetched Windows SDK, and JUCE 8.

---

## Prerequisites

| Tool | Install | Notes |
|------|---------|-------|
| cmake ≥ 3.22 | `apt install cmake` | |
| clang-18 | `apt install clang-18` | |
| lld-18 | `apt install lld-18` | Windows linker |
| llvm-18 | `apt install llvm-18` | Provides llvm-rc, llvm-mt, llvm-ar |
| ninja-build | `apt install ninja-build` | Required — make not supported |
| ccache | `apt install ccache` | Strongly recommended — see "Build speed" below |
| wine64 | `apt install wine64` | VST3 manifest step only |
| JUCE 8 | `git clone https://github.com/juce-framework/JUCE.git ~/JUCE` | |
| xwin | `cargo install xwin` | Requires Rust ≥ 1.80 |
| X11 dev headers | `apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libfreetype-dev libfontconfig-dev` | For native juceaide build |

**Note on Rust:** Ubuntu 24.04's packaged Rust (1.75) is too old for
current xwin. Install via rustup:
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source ~/.cargo/env
rustup update stable
cargo install xwin
```

---

## One-Time SDK Setup

### 1. Fetch Windows SDK

```bash
curl -sL "https://aka.ms/vs/17/release/channel" -o /tmp/vs17channel.json
xwin --accept-license --arch x86_64 --manifest /tmp/vs17channel.json \
     splat --output /opt/xwin-sdk
```

### 2. Case-fold symlinks (critical on Linux)

Linux is case-sensitive; SDK files use mixed case. Create lowercase symlinks:

```python
python3 << 'EOF'
import os
for d in ['/opt/xwin-sdk/sdk/lib/um/x86_64',
          '/opt/xwin-sdk/crt/lib/x86_64',
          '/opt/xwin-sdk/sdk/lib/ucrt/x86_64']:
    for f in os.listdir(d):
        lo = f.lower()
        if lo != f and not os.path.exists(os.path.join(d, lo)):
            os.symlink(os.path.join(d, f), os.path.join(d, lo))
EOF
```

Exact-case symlinks for specific libs:
```bash
cd /opt/xwin-sdk/sdk/lib/um/x86_64/
for pair in "DBGHELP.lib DbgHelp.lib" "DWRITE.lib Dwrite.lib" \
            "D2D1.lib D2d1.lib" "DCOMP.lib DComp.lib"; do
    ln -sf $(echo $pair | cut -d' ' -f1) $(echo $pair | cut -d' ' -f2) 2>/dev/null
done
```

DbgHelp.h case fix:
```bash
ln -sf /opt/xwin-sdk/sdk/include/10.0.26100/um/DbgHelp.h \
       /opt/xwin-sdk/sdk/include/10.0.26100/um/Dbghelp.h
```

### 3. Debug CRT stubs

CMake's compiler detection links against debug CRT libs that xwin doesn't
include. Create symlinks from the release versions:

```bash
for f in msvcrt msvcprt vcruntime libcmt libvcruntime; do
    ln -sf /opt/xwin-sdk/crt/lib/x86_64/${f}.lib \
           /opt/xwin-sdk/crt/lib/x86_64/${f}d.lib
done
ln -sf /opt/xwin-sdk/sdk/lib/ucrt/x86_64/ucrt.lib \
       /opt/xwin-sdk/sdk/lib/ucrt/x86_64/ucrtd.lib
```

Without these, CMake rejects the toolchain with the misleading error
"compiler not found" (it actually means "debug lib not found").

### 4. VS2022 STL Clang version check (if using Clang 18)

The VS2022 STL headers shipped via xwin require Clang 19+. If you're
using Clang 18, patch the version check:

```bash
sed -i 's/#if __clang_major__ < 19/#if __clang_major__ < 18/' \
    /opt/xwin-sdk/crt/include/yvals_core.h
```

This is safe — Clang 18 is fully compatible with the VS2022 STL; the
check is overly conservative.

### 5. Compiler wrapper and tool symlinks

clang-cl must be invoked with `--target=x86_64-pc-windows-msvc` and
CMake sometimes passes `--` as a separator that clang-cl doesn't
understand. Create a wrapper:

```bash
# Create the clang-cl symlink (argv[0] determines MSVC-compatibility mode)
ln -sf /usr/lib/llvm-18/bin/clang /usr/lib/llvm-18/bin/clang-cl

cat > /usr/local/bin/cricket-clang-cl << 'WRAP'
#!/bin/bash
args=()
for arg in "$@"; do [[ "$arg" == "--" ]] && continue; args+=("$arg"); done
exec /usr/lib/llvm-18/bin/clang-cl --target=x86_64-pc-windows-msvc "${args[@]}"
WRAP
chmod +x /usr/local/bin/cricket-clang-cl

ln -sf /usr/lib/llvm-18/bin/clang-cl /usr/local/bin/clang-cl
ln -sf /usr/bin/llvm-ar-18           /usr/local/bin/llvm-lib
ln -sf /usr/bin/lld-link-18          /usr/local/bin/lld-link
ln -sf /usr/lib/llvm-18/bin/lld-link /usr/bin/lld-link
ln -sf /usr/bin/llvm-mt-18           /usr/local/bin/llvm-mt
ln -sf /usr/bin/llvm-rc-18           /usr/local/bin/llvm-rc
```

**Important:** `/usr/bin/lld-link` must exist (not just `-18` suffix)
because the vst3_helper sub-cmake hardcodes this path.

---

## Build speed (ccache + build script)

**The single biggest iteration-time win is `ccache`.** Almost all of a
plugin's build time is recompiling JUCE's ~30 module translation units,
not your own source. Crucially, **bumping the version in `CMakeLists.txt`
and re-running `cmake` regenerates JUCE's generated header and compile
definitions, which invalidates those JUCE objects and forces a full
recompile.** Without a compiler cache, every version bump (and every other
reconfigure) pays the multi-minute JUCE rebuild again. With ccache the
identical JUCE compilations are served from cache in seconds.

Wire ccache in via CMake's compiler-launcher hook. Add this near the top
of `CMakeLists.txt`, **before** `add_subdirectory(... JUCE ...)`:

```cmake
# Serve JUCE module objects from cache across reconfigures (e.g. version
# bumps), so iterations stay fast.
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()
```

One-time: `apt install ccache && ccache -M 5G` (generous cache so a whole
JUCE tree fits). Measured effect: a full rebuild from an empty build
directory drops from several minutes to ~60s once the cache is warm; a
normal one-file source edit rebuilds in well under a minute.

Two further points:
- **Parallelism.** JUCE's translation units are independent, so always
  pass `-j` explicitly. In sandboxes where `nproc` reports 1, ninja
  otherwise serialises everything; oversubscribe with `-j4` (or more).
- **The ninja patches must be re-applied after *every* `cmake` run.**
  Since reconfigures are now cheap and frequent, do not apply them by
  hand — script them. See below.

### Reusable build script

Because the `rules.ninja` and `build.ninja` patches must be re-applied
after every configure, wrap configure + patch + build in one script
(`build.sh`) committed alongside the plugin. It configures only when
needed, re-applies all patches, and oversubscribes ninja:

```bash
#!/bin/bash
# build.sh — configure (if needed), re-apply all ninja patches, build.
# Usage: ./build.sh   |   ./build.sh reconfigure
set -e
ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD=$ROOT/build-win
TOOLCHAIN=$ROOT/clang-cl-toolchain.cmake
SDK=/opt/xwin-sdk/sdk/include/10.0.26100
CRT=/opt/xwin-sdk/crt/include
JOBS=$(nproc); [ "$JOBS" -lt 4 ] && JOBS=4
source "$HOME/.cargo/env" 2>/dev/null || true
mkdir -p "$BUILD"; cd "$BUILD"

NEED=0; [ "$1" = "reconfigure" ] && NEED=1; [ ! -f build.ninja ] && NEED=1
if [ "$NEED" = "1" ]; then
    cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DCMAKE_BUILD_TYPE=Release
    # rules.ninja: clang-cl rejects the "--" separator (compile + RC preproc)
    sed -i 's/ -c -- \$in/ -c \$in/g; s/ -E -- \$in/ -E \$in/g' \
        CMakeFiles/rules.ninja
    # build.ninja: vst3_helper toolchain, RC includes, wine prefix
    python3 - "$TOOLCHAIN" "$SDK" "$CRT" "$(basename "$ROOT")" << 'PY'
import sys
toolchain, sdk, crt, proj = sys.argv[1:5]
# NOTE: the rc_lib target name and the vst3_helper.exe path contain the
# *target* name (e.g. "Moth"), not the project dir — adjust if they differ.
c = open('build.ninja').read()
for old, new in [
  ('-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 && /usr/bin/cmake --build',
   f'-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 -DCMAKE_LINKER=/usr/local/bin/lld-link -DCMAKE_MT=/usr/local/bin/llvm-mt -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE={toolchain} && /usr/bin/cmake --build'),
  ('  FLAGS = -DWIN32\n  OBJECT_DIR = CMakeFiles/'+proj+'_rc_lib.dir',
   f'  FLAGS = -DWIN32\n  INCLUDES = -I{sdk}/um -I{sdk}/shared -I{crt} -I{sdk}/ucrt\n  OBJECT_DIR = CMakeFiles/'+proj+'_rc_lib.dir'),
  ('/JuceLibraryCode/vst3_helper/vst3_helper.exe > ',
   '/JuceLibraryCode/vst3_helper/vst3_helper.exe > '),  # see wine note
]:
    if old in c and old != new: c = c.replace(old, new)
# wine prefix (match the actual absolute exe path in your tree):
import re
c = re.sub(r'(&& )(\S*?/JuceLibraryCode/vst3_helper/vst3_helper\.exe > )',
           r'\1DISPLAY= WINEDEBUG=-all wine \2', c)
open('build.ninja','w').write(c)
print("patches applied")
PY
fi
ninja -j"$JOBS" ${PROJECT_TARGET:-${1:-}} "$@" 2>/dev/null || ninja -j"$JOBS"
```

In practice each plugin pins its own target name (e.g. `ninja -j$JOBS
Moth_VST3`); keep a per-project copy rather than over-generalising the
last line. The important, reusable parts are: ccache in `CMakeLists.txt`,
explicit `-j`, and scripting the patch re-application so a reconfigure is
a single command.



```bash
mkdir build-win && cd build-win
cmake .. -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=../clang-cl-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

### Patch rules.ninja (required after every cmake run)

CMake generates `CMakeFiles/rules.ninja` with ` -c -- $in` which
clang-cl doesn't understand:

```bash
sed -i 's/ -c -- \$in/ -c \$in/g' CMakeFiles/rules.ninja
```

### Patch build.ninja (required after every cmake run)

Three patches to `build.ninja`:

**A. vst3_helper toolchain forwarding:**
JUCE's VST3 build spawns a nested CMake for `vst3_helper.exe` that
doesn't inherit the parent toolchain. Find the line containing
`-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 && /usr/bin/cmake --build`
and insert before `&& /usr/bin/cmake --build`:

```
-DCMAKE_LINKER=/usr/local/bin/lld-link
-DCMAKE_MT=/usr/local/bin/llvm-mt
-DCMAKE_BUILD_TYPE=Release
-DCMAKE_TOOLCHAIN_FILE=/path/to/clang-cl-toolchain.cmake
```

**B. RC compiler include paths:**
Find the `DragonflyFX_rc_lib` build target and add an `INCLUDES` line:

```
INCLUDES = -I/opt/xwin-sdk/sdk/include/10.0.26100/um -I/opt/xwin-sdk/sdk/include/10.0.26100/shared -I/opt/xwin-sdk/crt/include -I/opt/xwin-sdk/sdk/include/10.0.26100/ucrt
```

**C. Wine prefix for vst3_helper.exe:**
Find the line that runs `vst3_helper.exe > moduleinfo.json` and prefix
the exe path with `DISPLAY= WINEDEBUG=-all wine `.

All three patches can be applied with this Python script:

```python
import sys
with open('build.ninja', 'r') as f:
    c = f.read()
toolchain = '/path/to/clang-cl-toolchain.cmake'  # adjust
patches = [
    ('-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 && /usr/bin/cmake --build',
     f'-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 -DCMAKE_LINKER=/usr/local/bin/lld-link -DCMAKE_MT=/usr/local/bin/llvm-mt -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE={toolchain} && /usr/bin/cmake --build'),
    ('  FLAGS = -DWIN32\n  OBJECT_DIR = CMakeFiles/DragonflyFX_rc_lib.dir',
     '  FLAGS = -DWIN32\n  INCLUDES = -I/opt/xwin-sdk/sdk/include/10.0.26100/um -I/opt/xwin-sdk/sdk/include/10.0.26100/shared -I/opt/xwin-sdk/crt/include -I/opt/xwin-sdk/sdk/include/10.0.26100/ucrt\n  OBJECT_DIR = CMakeFiles/DragonflyFX_rc_lib.dir'),
    ('DragonflyFX_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe > ',
     'DISPLAY= WINEDEBUG=-all wine DragonflyFX_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe > '),
]
for old, new in patches:
    if old in c: c = c.replace(old, new); print(f"OK: {old[:50]}...")
    else: print(f"MISS: {old[:50]}...", file=sys.stderr)
with open('build.ninja', 'w') as f:
    f.write(c)
```

### Build

```bash
ninja DragonflyFX_VST3
```

### Output

```
build-win/DragonflyFX_artefacts/Release/VST3/Dragonfly FX.vst3/
  Contents/
    x86_64-win/Dragonfly FX.vst3   ← PE32+ DLL
    Resources/moduleinfo.json       ← VST3 manifest
```

Install by copying the `Dragonfly FX.vst3` folder to
`C:\Program Files\Common Files\VST3\` on Windows.

---

## Troubleshooting

| Error | Cause | Fix |
|-------|-------|-----|
| `cannot open file 'dbghelp.lib'` | Case mismatch | Create `DbgHelp.lib` symlink |
| `unknown argument '--'` | rules.ninja not patched | `sed -i 's/ -c -- \$in/ -c \$in/g'` |
| `LNK1104: cannot open 'msvcrtd.lib'` | Debug CRT missing | Create debug CRT symlinks |
| `compiler not found` (cmake) | Same as above | Create debug CRT symlinks |
| `STL1000: Unexpected compiler version` | Clang 18 vs STL check | Patch yvals_core.h |
| `windows.h not found` (llvm-rc) | RC missing includes | Patch build.ninja with SDK include paths |
| VST3 build hangs at moduleinfo | vst3_helper not under Wine | Patch build.ninja with wine prefix |
| VST3 sub-cmake linker error | Nested cmake missing toolchain | Patch build.ninja with toolchain flags |
| `lld-link: not found` | Missing /usr/bin/lld-link | Create symlink from lld-link-18 |
| `X11/extensions/Xrandr.h not found` | juceaide needs X11 headers | Install libxrandr-dev etc. |

---

## Source Layout

```
CMakeLists.txt               JUCE plugin definition, targets, version
clang-cl-toolchain.cmake     Windows cross-compile toolchain
CROSS_COMPILE_BUILD.md       This file
Source/
  FXEngine.h                 DSP: MS-20 SVF filter, Comb, Chebyshev waveshaper,
                             LFO, Envelope follower
  PluginProcessor.h/.cpp     JUCE AudioProcessor, APVTS parameters, audio loop
  PluginEditor.h/.cpp        UI: FX block panels, LFO panels, Env panel, L&F
```
