# Silkworm — Build Guide (v1.0)

Cross-compiles Windows x86-64 VST3 + standalone from Linux (Ubuntu 24.04)
using the same toolchain as Cricket / Dragonfly FX.

---

## Prerequisites

Same toolchain as Cricket — see `CROSS_COMPILE_BUILD.md` for full one-time
setup (xwin SDK, case-fold symlinks, debug CRT stubs, clang-cl wrapper,
yvals_core.h patch).

| Tool | Notes |
|------|-------|
| cmake ≥ 3.22 | |
| clang-18 + lld-18 + llvm-18 | |
| ninja-build | |
| wine64 | VST3 manifest step only |
| JUCE 8 at `~/JUCE` | |
| xwin SDK at `/opt/xwin-sdk` | |
| cricket-clang-cl wrapper | Shared with Cricket |

---

## Building

### Configure

```bash
cd silkworm
mkdir build-win && cd build-win

cmake .. -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=../clang-cl-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

### Patch rules.ninja (required after every cmake run)

```bash
sed -i 's/ -c -- \$in/ -c \$in/g' CMakeFiles/rules.ninja
```

### Build standalone (no Wine needed)

```bash
ninja Silkworm_Standalone
```

### Build VST3 (needs build.ninja patches + Wine)

Apply three patches to `build.ninja`:

```python
import sys

with open('build.ninja', 'r') as f:
    c = f.read()

toolchain = '../clang-cl-toolchain.cmake'

patches = [
    # A. Forward toolchain to vst3_helper sub-cmake
    ('-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18 && /usr/bin/cmake --build',
     f'-DCMAKE_RC_COMPILER=/usr/bin/llvm-rc-18'
     f' -DCMAKE_LINKER=/usr/local/bin/lld-link'
     f' -DCMAKE_MT=/usr/local/bin/llvm-mt'
     f' -DCMAKE_BUILD_TYPE=Release'
     f' -DCMAKE_TOOLCHAIN_FILE={toolchain}'
     f' && /usr/bin/cmake --build'),

    # B. RC compiler include paths
    ('  FLAGS = -DWIN32\n  OBJECT_DIR = CMakeFiles/Silkworm_rc_lib.dir',
     '  FLAGS = -DWIN32\n'
     '  INCLUDES = -I/opt/xwin-sdk/sdk/include/10.0.26100/um'
     ' -I/opt/xwin-sdk/sdk/include/10.0.26100/shared'
     ' -I/opt/xwin-sdk/crt/include'
     ' -I/opt/xwin-sdk/sdk/include/10.0.26100/ucrt\n'
     '  OBJECT_DIR = CMakeFiles/Silkworm_rc_lib.dir'),

    # C. Run vst3_helper under Wine
    ('Silkworm_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe > ',
     'DISPLAY= WINEDEBUG=-all wine '
     'Silkworm_artefacts/JuceLibraryCode/vst3_helper/vst3_helper.exe > '),
]

for old, new in patches:
    if old in c:
        c = c.replace(old, new)
        print(f"OK: {old[:60]}...")
    else:
        print(f"MISS: {old[:60]}...", file=sys.stderr)

with open('build.ninja', 'w') as f:
    f.write(c)
```

Then build:

```bash
ninja Silkworm_VST3
```

### Clear stale vst3_helper cache (if sub-cmake fails)

```bash
rm -rf Silkworm_artefacts/JuceLibraryCode/vst3_helper/CMakeCache.txt \
       Silkworm_artefacts/JuceLibraryCode/vst3_helper/CMakeFiles
```

---

## Output

```
build-win/Silkworm_artefacts/Release/Standalone/Silkworm.exe
build-win/Silkworm_artefacts/Release/VST3/Silkworm.vst3/
  Contents/
    x86_64-win/Silkworm.vst3        ← PE32+ DLL
    Resources/moduleinfo.json        ← VST3 manifest
```

Install the VST3 by copying the `Silkworm.vst3` folder to
`C:\Program Files\Common Files\VST3\` on Windows.

---

## Source Layout

```
CMakeLists.txt               JUCE plugin definition, targets, version
clang-cl-toolchain.cmake     Windows cross-compile toolchain
BUILD.md                     This file
Source/
  SilkwormEngine.h           DSP: allpass loop reverb, bit-crush, clock decimator
  PluginProcessor.h/.cpp     JUCE AudioProcessor, APVTS parameters, audio loop
  PluginEditor.h/.cpp        UI: panels, knobs, look-and-feel
```
