#!/usr/bin/env bash
# Dragonfly FX — Windows x64 VST3 cross-build (clang-cl 18 + xwin SDK + JUCE 8). See CROSS_COMPILE_BUILD.md for one-time SDK setup.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
BUILD="${BUILD:-$HERE/build-win}"
[ -f /root/JUCE/CMakeLists.txt ] || { echo "JUCE 8 must be at /root/JUCE (git clone --branch 8.0.15 https://github.com/juce-framework/JUCE.git)"; exit 1; }
cmake -S "$HERE" -B "$BUILD" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$HERE/clang-cl-toolchain.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
python3 "$HERE/patch_ninja.py" "$BUILD" "$HERE/clang-cl-toolchain.cmake"   # rules.ninja '-c --', vst3_helper toolchain, RC includes, Wine
ninja -C "$BUILD" DragonflyFX_VST3
echo "Built: $BUILD/DragonflyFX_artefacts/Release/VST3/Dragonfly FX.vst3"
