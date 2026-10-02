# Benzene

A 1950s-style **diode-ring modulator** VST3 plugin for Windows, cross-compiled
from Linux. Benzene pairs a tube-style internal carrier oscillator with a
diode-ring modulator modelled for a raw, gritty, slightly-unstable character,
plus a three-source modulation matrix.

Current version: **2.1.0**

---

## Sound & signal flow

```
            ┌─────────────────────┐
  input ───►│   diode ring mod    ├──► mix ──► output
            │  (waveshaper model) │
   ┌────────┤  ▲                  │
   │ carrier│  │ carrier          │
   │  osc   ├──┘                  │
   └────────┘                     │
        ▲                         ▲
        └──── modulation matrix ──┘
       (LFO / pitch tracker / envelope follower)
```

### Signal generator (carrier)
A tube-style oscillator with seven waveforms — **Sine, Triangle, Sawtooth,
Pulse, White, Pink, LFSR** — plus:
- **Drive** — asymmetric triode-style saturation (adds even harmonics).
- **Drift** — context-dependent: slow pitch wander on tonal waves; a lowpass
  tone control on White/Pink noise; LFSR **bit depth (3–16 bits)** — which sets
  the shift-register cycle length, from short buzzy pitched noise to long rumble.
- **Symmetry** (bipolar) — per-waveform: triangle peak position, pulse width
  (clamped ~5–72% so it always sounds), saw ramp warp, sine phase-distortion
  skew, noise distribution skew, LFSR sample-rate/bit reduction.

### Ring modulator
A **stateless waveshaper** model (after Parker, DAFx-11): the diode pairs are
two static shaping curves fed by `carrier ± input`; their difference is the
ring-modulated output. Unconditionally stable, cheap, no ODE solver.
- **Diode type** — Germanium (soft, rounded) or Silicon (harder, brighter).
- **Imbalance** — purely tonal diode-asymmetry/harmonics (no dry bleed).
- **Instability** — bounded "controlled chaos": wanders the diode thresholds and
  adds intermodulation fizz, never diverges.
- **Stability** — oversampling (1×/2×/4×/8×); lower = grittier.
- **Input / Carrier** levels.

### Output & gate
- **Mix**, **Output**.
- **Gate** — when on, the wet path is silent with no input (no idle carrier
  bleed). When off, the carrier routes straight out regardless of Imbalance, so
  Benzene works as a standalone oscillator / drone source.

### Modulation (three sources × eight destinations)
Three horizontal strips below the main panels. Each source has a bipolar depth
knob for every continuous destination (FREQ, DRIVE, DRIFT, SYM, IMBAL, INST,
INPUT, CARR). Per-destination lights show which routings are active and pulse
with the live modulation amount.
- **LFO** — Rate (0.02–40 Hz) + Shape (continuous triangle→saw→square blend).
- **Pitch Tracker** — zero-crossing pitch estimate of the input, with Sensitivity
  (bipolar around a 220 Hz reference, ±2 octaves full scale).
- **Envelope Follower** — input amplitude, with Sensitivity and Speed.

Modulation is computed once per audio block and applied to the base parameter
values (frequency multiplicatively in octaves; others additively with clamping).

---

## Source layout

```
CMakeLists.txt              JUCE plugin definition, targets, version
clang-cl-toolchain.cmake    Windows cross-compile toolchain for CMake
SETUP.sh                    one-time toolchain + Windows SDK install
build.sh                    (re)build the VST3
Source/
  BenzeneEngine.h           carrier oscillator + stateless diode-ring modulator
  ModEngine.h               LFO, pitch tracker, envelope follower
  PluginProcessor.h/.cpp    AudioProcessor, APVTS params, mod matrix, audio loop
  PluginEditor.h/.cpp       UI: panels, knobs, look-and-feel, mod strips
tools/
  editor_snapshot.cpp       headless editor instantiation + PNG render (CI/debug)
```

---

## Building

Target: **Windows x86-64 VST3**, built from **Linux** (tested on Ubuntu 24.04)
with clang-cl 18, the xwin-fetched Windows SDK, JUCE 8, Ninja, and Wine.

```bash
# 1. one-time toolchain + Windows SDK setup
./SETUP.sh

# 2. build (re-runnable; re-applies the per-configure Ninja patches)
./build.sh
```

Output:
```
build-win/Benzene_artefacts/Release/VST3/Benzene.vst3/
```
Copy that `Benzene.vst3` folder to your Windows VST3 directory, e.g.
`C:\Program Files\Common Files\VST3\`.

Overrides: `JUCE_DIR` (default `$HOME/JUCE`), `XWIN` (default `/opt/xwin-sdk`),
`BUILD_DIR` (default `build-win`). `./build.sh clean` wipes and rebuilds.

### Why the build needs patches
JUCE's VST3 build and CMake/Ninja's clang-cl handling need a few fixups that
must be re-applied **after every `cmake` configure** (Ninja regenerates the
files each time). `build.sh` does this automatically:
1. `rules.ninja` — strip a `--` separator clang-cl rejects.
2. `build.ninja` — forward the toolchain to JUCE's nested `vst3_helper` CMake.
3. `build.ninja` — add Windows SDK include paths to the `.rc` resource compile.
4. `build.ninja` — run `vst3_helper.exe` under Wine to emit `moduleinfo.json`.

See comments in `SETUP.sh` and `build.sh` for the full detail (case-folding
symlinks, debug-CRT stubs, STL Clang-version guard, etc.).

### Editor snapshot test (optional)
`tools/editor_snapshot.cpp` instantiates the processor and editor headlessly and
renders the UI to a PNG. It catches the common class of editor-construction
crashes (and layout regressions) on Linux in seconds, without a Windows host.
Build it as a JUCE GUI app against the same sources and run under `xvfb-run`.

---

## Notes
- Source is provided as-is; the primary deliverable across development was the
  compiled VST3.
- The ring-modulator model is intentionally a simple, stable approximation
  rather than a full circuit ODE simulation — chosen after the ODE approach
  proved both CPU-heavy and prone to dropouts. The trade-off is documented in
  the engine header comments.
