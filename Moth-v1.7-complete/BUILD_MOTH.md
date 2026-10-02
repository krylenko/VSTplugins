# Moth — Build Guide (v1.6)

Moth is a Windows x86-64 VST3 audio effect: a ProCo RAT-style distortion
feeding a Schulte/Krautrock-style 6-stage all-pass phaser. It is
cross-compiled from Linux (Ubuntu 24.04) with clang-cl 18, the
xwin-fetched Windows SDK, and JUCE 8.

For the full background on the cross-compile toolchain and the
non-obvious problems it solves, see `CROSS_COMPILE_BUILD.md`. This file
is the short, Moth-specific version.

--------------------------------------------------------------------
## Source layout
--------------------------------------------------------------------
    CMakeLists.txt              JUCE plugin definition, version, targets
    clang-cl-toolchain.cmake    Windows cross-compile toolchain
    build.sh                    Configure + patch + build in one command
    Source/
      MothEngine.h              DSP: RAT distortion + Krautrock phaser
      PluginProcessor.h/.cpp    JUCE AudioProcessor, APVTS parameters
      PluginEditor.h/.cpp       UI: panels, knobs, CRT look-and-feel

--------------------------------------------------------------------
## Prerequisites (one-time)
--------------------------------------------------------------------
    apt install cmake clang-18 lld-18 llvm-18 ninja-build ccache \
                wine64 libx11-dev libxrandr-dev libxinerama-dev \
                libxcursor-dev libfreetype-dev libfontconfig-dev \
                libasound2-dev libxcomposite-dev

    # JUCE 8 at /home/claude/JUCE (or edit the path in CMakeLists.txt)
    git clone --depth 1 https://github.com/juce-framework/JUCE.git /home/claude/JUCE

    # Rust + xwin (fetches the Windows SDK)
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source ~/.cargo/env
    cargo install xwin

    ccache -M 5G    # generous compiler cache

### Windows SDK via xwin
    curl -sL "https://aka.ms/vs/17/release/channel" -o /tmp/vs17channel.json
    xwin --accept-license --arch x86_64 --manifest /tmp/vs17channel.json \
         splat --output /opt/xwin-sdk

Then apply the SDK fix-ups (case-fold symlinks, debug-CRT stubs,
DbgHelp.h case fix, and the Clang-18 STL version patch). These are all
documented with copy-paste blocks in `CROSS_COMPILE_BUILD.md`
(sections "One-Time SDK Setup" steps 2–4). The key one for Clang 18:

    sed -i 's/#if __clang_major__ < 19/#if __clang_major__ < 18/' \
        /opt/xwin-sdk/crt/include/yvals_core.h

### Compiler wrapper + tool symlinks
The toolchain expects a clang-cl wrapper at
`/usr/local/bin/cricket-clang-cl` plus a handful of tool symlinks
(`clang-cl`, `llvm-lib`, `lld-link`, `llvm-mt`, `llvm-rc`). The exact
commands are in `CROSS_COMPILE_BUILD.md` ("Compiler wrapper and tool
symlinks").

--------------------------------------------------------------------
## Building
--------------------------------------------------------------------
Everything (configure, the required ninja patches, and the build) is
wrapped in `build.sh`:

    ./build.sh reconfigure     # first build, or after editing CMakeLists
    ./build.sh                 # incremental build after a source edit

`build.sh` re-applies all five ninja patches automatically after every
configure (the `rules.ninja` `--` fixes and the three `build.ninja`
VST3 patches), so you never apply them by hand. With ccache warm, a
full rebuild is ~60–90s and a one-file edit is well under a minute.

### Output
    build-win/Moth_artefacts/Release/VST3/Moth.vst3/
      Contents/
        x86_64-win/Moth.vst3        ← PE32+ DLL
        Resources/moduleinfo.json   ← VST3 manifest

--------------------------------------------------------------------
## Install on Windows
--------------------------------------------------------------------
Copy the whole `Moth.vst3` folder to:

    C:\Program Files\Common Files\VST3\

then rescan plugins in your DAW.

--------------------------------------------------------------------
## Versioning
--------------------------------------------------------------------
Version lives in `CMakeLists.txt` (two places: the `project()` line and
the `juce_add_plugin VERSION` line). It is shown in small text in the
plugin's header. Bump the minor version for each new iteration.
Current: 1.6.0.
