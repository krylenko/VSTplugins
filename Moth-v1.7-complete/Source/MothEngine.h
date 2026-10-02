#pragma once
#include <cmath>
#include <array>
#include <random>

// ============================================================
//  MothEngine.h
//  Two DSP sections, processed in series:
//    1. RatDistortion  — ProCo RAT-style overdrive/distortion
//    2. KrautPhaser    — thick, gritty 6-stage all-pass phaser
//                        modeled on the Schulte Compact Phasing A
//                        ("Krautrock phaser"), heavy feedback,
//                        nonlinear LDR-style sweep
// ============================================================

namespace moth {

static constexpr float PI_F = 3.14159265358979323846f;

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// ------------------------------------------------------------
//  One-pole smoother for click-free parameter changes
// ------------------------------------------------------------
struct SmoothedValue {
    float current = 0.0f;
    float target  = 0.0f;
    float coeff   = 0.0f;   // per-sample

    void setTimeConstant(float ms, double sr) {
        // coeff = exp(-1 / (ms * 0.001 * sr))
        float samples = (float)(ms * 0.001 * sr);
        if (samples < 1.0f) samples = 1.0f;
        coeff = std::exp(-1.0f / samples);
    }
    void setImmediate(float v) { current = target = v; }
    inline float next() {
        current = target + (current - target) * coeff;
        return current;
    }
};

// ============================================================
//  RatDistortion
//  Signal chain modeled on the ProCo RAT:
//    input HPF -> high-gain stage (LM308 slew-limited) ->
//    hard diode clip (~±0.65V) -> post "Filter" lowpass ->
//    output volume
//
//  Controls:
//    distortion  0..1   (maps to op-amp gain, ~ x2 .. x1000)
//    filter      0..1   (RAT "Filter": CW = darker. Here 0=dark,
//                        1=bright so the knob reads intuitively;
//                        the processor inverts to match a real RAT
//                        if desired — we expose 0=dark..1=bright)
//    volume      0..1
// ============================================================
class RatDistortion {
public:
    void prepare(double sampleRate) {
        sr = sampleRate;

        // Input high-pass ~ 30 Hz (RAT input coupling / Ruetz region):
        // keeps lows from muddying the clipper but stays fairly full.
        hpCoeff = std::exp(-2.0f * PI_F * 31.0f / (float)sr);
        hpX = hpY = 0.0f;

        // LM308 slew-rate limiting. The real 308 has a very low slew
        // rate (~0.3 V/us uncompensated; the RAT's comp cap slows the
        // edge). We emulate this as a slew clamp on the pre-clip signal,
        // which gives the RAT its rounded, compressed grind.
        // Convert a notional volts/sec into per-sample max delta.
        slewPerSample = (float)(slewVoltsPerSec / sr);

        // Post-clip "Filter" one-pole — coefficient computed per block.
        fltY = 0.0f;

        // Mild output DC blocker
        dcX = dcY = 0.0f;
        dcCoeff = std::exp(-2.0f * PI_F * 12.0f / (float)sr);

        gainSm.setTimeConstant(20.0f, sr);
        fltSm.setTimeConstant(20.0f, sr);
        volSm.setTimeConstant(20.0f, sr);
        slewState = 0.0f;
    }

    void setParams(float distortion01, float filter01, float volume01) {
        // Op-amp gain: RAT gain ~ 1 + Rf/(47 + Rdist). With pot at 0 the
        // gain is modest; cranked it is enormous. Map exponentially.
        // ~x1.5 at 0 to ~x900 at 1.
        float g = 1.5f * std::pow(600.0f, distortion01);
        gainSm.target = g;

        // Filter: map 0..1 to a post-clip lowpass cutoff.
        // 0 -> ~600 Hz (dark, classic cranked-RAT),
        // 1 -> ~16 kHz (wide open, fizzy — passes the clipped
        //      signal's upper harmonics for a brighter, edgier voice).
        float fc = 600.0f * std::pow(16000.0f / 600.0f, filter01);
        fltSm.target = fc;

        // Volume with a gentle taper. RAT output is hot; keep unity-ish.
        volSm.target = volume01 * volume01 * 1.2f;
    }

    inline float processSample(float x) {
        // ---- input high-pass ----
        float hp = hpCoeff * (hpY + x - hpX);
        hpX = x;
        hpY = hp;

        // ---- gain stage ----
        float g = gainSm.next();
        float driven = hp * g;

        // ---- LM308 slew limiting (asymmetric-ish soft compression) ----
        float delta = driven - slewState;
        float maxStep = slewPerSample * g * 0.02f + slewPerSample;
        if (delta >  maxStep) delta =  maxStep;
        if (delta < -maxStep) delta = -maxStep;
        slewState += delta;
        float pre = slewState;

        // ---- hard diode clipping to ~±0.65 V ----
        // Two-silicon-diode hard clip with a small soft knee for realism.
        const float clip = 0.65f;
        float clipped;
        if (pre > clip)        clipped = clip + std::tanh((pre - clip) * 0.6f) * 0.06f;
        else if (pre < -clip)  clipped = -clip + std::tanh((pre + clip) * 0.6f) * 0.06f;
        else                   clipped = pre;

        // ---- post-clip "Filter" lowpass ----
        float fc = fltSm.next();
        float a = std::exp(-2.0f * PI_F * fc / (float)sr);
        fltY = a * fltY + (1.0f - a) * clipped;

        // ---- DC block + volume ----
        float dc = fltY - dcX + dcCoeff * dcY;
        dcX = fltY;
        dcY = dc;

        return dc * volSm.next();
    }

private:
    double sr = 44100.0;

    float hpCoeff = 0.0f, hpX = 0.0f, hpY = 0.0f;
    float fltY = 0.0f;
    float dcX = 0.0f, dcY = 0.0f, dcCoeff = 0.0f;

    float slewState = 0.0f;
    float slewPerSample = 0.0f;
    static constexpr double slewVoltsPerSec = 1400.0; // tuned for grind

    SmoothedValue gainSm, fltSm, volSm;
};

// ============================================================
//  PhaserLFO
//  Selectable shapes: 0=Sine/Tri (smooth), 1=Triangle, 2=Square,
//  3=Sawtooth, 4=Random (sample+hold with slew).
//  Outputs 0..1.
// ============================================================
class PhaserLFO {
public:
    enum Shape { SINE = 0, TRIANGLE, SQUARE, SAW, RANDOM };

    void prepare(double sampleRate) {
        sr = sampleRate;
        phase = 0.0f;
        rng.seed(0xC0FFEEu);
        shCurrent = nextRand();
        shTarget  = nextRand();
        shSlew = 0.0f;
    }

    void setRate(float hz) { rateHz = hz; }
    void setShape(int s)   { shape = s; }

    inline float next() {
        float inc = rateHz / (float)sr;
        phase += inc;
        while (phase >= 1.0f) { phase -= 1.0f; advanceRandom(); }
        return shapeAt(phase);
    }

    // Continuous read of the same LFO at a fixed phase offset (0..1).
    // Used for the stereo channel so its sweep is a genuine phase-shifted
    // copy — NOT a value offset, which would wrap and click once per cycle.
    inline float valueAtOffset(float phaseOffset) const {
        float p = phase + phaseOffset;
        while (p >= 1.0f) p -= 1.0f;
        return shapeAt(p);
    }

private:
    // Evaluate the current waveform at an arbitrary phase (0..1).
    inline float shapeAt(float p) const {
        float out = 0.0f;
        switch (shape) {
            case SINE:
                out = 0.5f - 0.5f * std::cos(2.0f * PI_F * p);
                break;
            case TRIANGLE:
                out = p < 0.5f ? (p * 2.0f) : (2.0f - p * 2.0f);
                break;
            case SQUARE:
                // Smoothstep the edges so the square sweeps rather than
                // clicks; symmetric on both transitions, so continuous.
                {
                    const float edge = 0.02f;   // fraction of cycle for the ramp
                    if (p < edge)              out = smoothstep(p / edge);
                    else if (p < 0.5f)         out = 1.0f;
                    else if (p < 0.5f + edge)  out = 1.0f - smoothstep((p - 0.5f) / edge);
                    else                       out = 0.0f;
                }
                break;
            case SAW:
                out = p;
                break;
            case RANDOM:
                out = shCurrent + (shTarget - shCurrent) * smoothstep(p);
                break;
        }
        return clampf(out, 0.0f, 1.0f);
    }

    inline void advanceRandom() {
        shCurrent = shTarget;
        shTarget  = nextRand();
    }

    static float smoothstep(float t) { return t * t * (3.0f - 2.0f * t); }
    float nextRand() {
        std::uniform_real_distribution<float> d(0.0f, 1.0f);
        return d(rng);
    }

    double sr = 44100.0;
    float phase = 0.0f;
    float rateHz = 0.5f;
    int   shape = SINE;

    std::mt19937 rng;
    float shCurrent = 0.0f, shTarget = 0.0f, shSlew = 0.0f;
};

// ============================================================
//  AllpassStage — first-order all-pass, coefficient set per sample
//  from the swept centre frequency.
//    H(z) = (a + z^-1) / (1 + a z^-1)
//  Implemented as a transposed direct-form II so that summing the
//  output with the dry signal produces a moving notch (the phaser
//  effect). y[n] = a*x[n] + s ;  s = x[n] - a*y[n].
// ============================================================
struct AllpassStage {
    float state = 0.0f;
    inline float process(float x, float a) {
        float y = a * x + state;
        state   = x - a * y;
        return y;
    }
    void reset() { state = 0.0f; }
};

// ============================================================
//  KrautPhaser
//  6-stage all-pass phaser with strong feedback (regeneration),
//  LDR-style nonlinear frequency warping, and a "character" control
//  that morphs the fundamental phaser flavour:
//
//   character 0.00 .. 0.33  : classic 4-stage-ish OTA voice (gentler
//                             notches, narrower sweep) — but never clean
//   character 0.33 .. 0.66  : full 6-stage Schulte voice (wide, thick)
//   character 0.66 .. 1.00  : extreme regeneration, resonant / filter-like
//                             "almost self-oscillating" Krautrock zone
//
//  The LDR nonlinearity makes the sweep asymmetric and slightly
//  unstable, which is the gritty, "not hi-fi" part of the sound.
//
//  Stereo: the two channels sweep with a phase offset for the
//  wide, three-dimensional image the Schulte is known for.
// ============================================================
class KrautPhaser {
public:
    void prepare(double sampleRate) {
        sr = sampleRate;
        for (auto& s : stagesL) s.reset();
        for (auto& s : stagesR) s.reset();
        lfo.prepare(sampleRate);
        fbL = fbR = 0.0f;
        sweepSm.setTimeConstant(2.0f, sr);
        depthSm.setTimeConstant(15.0f, sr);
        fbSm.setTimeConstant(15.0f, sr);
        ldrStateL = ldrStateR = 0.5f;
        // LDR lag: photocells respond slowly (a few ms). This is what
        // smears the LFO into the gritty, non-instant sweep.
        ldrCoeff = std::exp(-1.0f / (0.006f * (float)sr)); // ~6ms
        // DC blockers on the feedback paths. Without these, the asymmetric
        // feedback saturation injects a DC offset that accumulates in the
        // loop and biases it into the flat region of the saturator, which
        // chokes the resonance at high feedback. ~5 Hz cutoff.
        fbDcCoeff = std::exp(-2.0f * PI_F * 5.0f / (float)sr);
        dcXL = dcYL = dcXR = dcYR = 0.0f;
    }

    void setParams(float speedHz, float depth01, float character01,
                   float feedback01, int lfoShape) {
        lfo.setRate(speedHz);
        lfo.setShape(lfoShape);
        depthSm.target = depth01;
        character = character01;
        // Loop feedback amount. The in-loop saturator has unity small-signal
        // gain, so the self-oscillation threshold is governed by this alone.
        // Base 0.78 (+character) keeps the loop stable across most of the
        // knob and only lets it tip into self-oscillation near the very top —
        // the authentic Krautrock "pinned = it howls" behaviour.
        float fbMax = 0.78f + 0.18f * character;       // up to ~0.96 at max char
        fbAmount = clampf(feedback01, 0.0f, 1.0f);      // raw, drives grit + loop sat
        fbSm.target = fbAmount * fbMax;

        // Number of "active" stages morphs with character (4 -> 6).
        activeStages = (character < 0.33f) ? 4 : 6;

        // Sweep span & centre move with character: narrow+low for the
        // gentle voice, wide for the thick voice.
        centreHz = 300.0f + 200.0f * character;
        spanOct  = 2.2f + 2.6f * character;            // octaves of sweep
    }

    // Process one stereo sample pair in place.
    inline void processStereo(float& L, float& R) {
        float lfoV = lfo.next();                 // 0..1
        float depth = depthSm.next();
        float fb    = fbSm.next();

        // LDR-style nonlinear warp of the control voltage: photocells
        // have a curved, asymmetric response and lag. Warp then lag.
        float warpedL = ldrWarp(lfoV);
        // Right channel: read the LFO at a quarter-cycle phase offset for a
        // wide stereo image. This is a true phase shift (continuous across
        // the cycle), not a value offset — so it doesn't click each cycle.
        float lfoR = lfo.valueAtOffset(0.25f);
        float warpedR = ldrWarp(lfoR);

        ldrStateL = ldrCoeff * ldrStateL + (1.0f - ldrCoeff) * warpedL;
        ldrStateR = ldrCoeff * ldrStateR + (1.0f - ldrCoeff) * warpedR;

        float fcL = sweepToHz(ldrStateL, depth);
        float fcR = sweepToHz(ldrStateR, depth);

        float aL = freqToCoeff(fcL);
        float aR = freqToCoeff(fcR);

        // ---- Left ----
        // DC-block the feedback before it re-enters the loop (keeps any tiny
        // offset from accumulating in the resonant loop).
        float fbInL = fbL - dcXL + fbDcCoeff * dcYL;
        dcXL = fbL; dcYL = fbInL;
        float xL = L + fbInL * fb;
        xL = loopSat(xL);                       // unity-gain: stable loop
        for (int i = 0; i < activeStages; ++i) xL = stagesL[i].process(xL, aL);
        fbL = xL;
        float wetL = gritShape(xL);             // grit added OUTSIDE the loop

        // ---- Right ----
        float fbInR = fbR - dcXR + fbDcCoeff * dcYR;
        dcXR = fbR; dcYR = fbInR;
        float xR = R + fbInR * fb;
        xR = loopSat(xR);
        for (int i = 0; i < activeStages; ++i) xR = stagesR[i].process(xR, aR);
        fbR = xR;
        float wetR = gritShape(xR);

        // Mix: phaser is the all-pass output summed with dry. The notches
        // come from this summation. Equal mix gives deepest notches.
        L = 0.5f * L + 0.5f * wetL;
        R = 0.5f * R + 0.5f * wetR;
    }

private:
    // In-loop saturator: UNITY small-signal gain (slope 1 at the origin), so
    // loop stability — and thus the self-oscillation threshold — is set purely
    // by the feedback coefficient, not by drive. Drive shapes large signals
    // (mild analog softening) without raising loop gain, so the loop does not
    // self-oscillate until feedback is near maximum.
    inline float loopSat(float x) {
        float drive = 1.0f + fbAmount * 3.0f;
        return std::tanh(x * drive) / drive;
    }

    // Out-of-loop grit: applied to the wet (all-pass) output before mixing,
    // NOT inside the feedback loop, so it adds harmonic dirt that grows with
    // the Feedback knob without affecting loop stability. Asymmetric for even
    // harmonics. This is what makes high feedback growl rather than ring.
    inline float gritShape(float x) {
        if (fbAmount < 0.001f) return x;
        float drive = 1.0f + fbAmount * 8.0f;
        float y = x * drive;
        float s = (y >= 0.0f) ? std::tanh(y) : std::tanh(y * 0.65f);
        float m = fbAmount;                 // dry->shaped blend grows with fb
        return s * m + x * (1.0f - m);
    }

    // LDR nonlinear response: curve + slight asymmetry. Input/out 0..1.
    inline float ldrWarp(float v) {
        // gamma curve (photocell-ish) + a touch of asymmetry
        float g = std::pow(v, 1.0f + 0.7f * character);
        // asymmetric kink: rising edge faster than falling near top
        g = g + 0.06f * std::sin(v * PI_F) * character;
        return clampf(g, 0.0f, 1.0f);
    }

    inline float sweepToHz(float ctrl01, float depth) {
        // centre +/- (depth * spanOct/2) octaves
        float oct = (ctrl01 - 0.5f) * spanOct * depth;
        float hz = centreHz * std::pow(2.0f, oct);
        return clampf(hz, 40.0f, 11000.0f);
    }

    inline float freqToCoeff(float fc) {
        // First-order all-pass coefficient for break freq fc:
        // a = (tan(pi*fc/sr) - 1) / (tan(pi*fc/sr) + 1)
        float t = std::tan(PI_F * fc / (float)sr);
        return (t - 1.0f) / (t + 1.0f);
    }

    double sr = 44100.0;
    std::array<AllpassStage, 6> stagesL, stagesR;
    PhaserLFO lfo;
    float fbL = 0.0f, fbR = 0.0f;
    int   activeStages = 6;
    float character = 0.5f;
    float fbAmount = 0.0f;   // raw feedback knob 0..1, drives fbSaturate
    float centreHz = 400.0f;
    float spanOct  = 3.5f;

    float ldrStateL = 0.5f, ldrStateR = 0.5f, ldrCoeff = 0.0f;
    float fbDcCoeff = 0.0f;
    float dcXL = 0.0f, dcYL = 0.0f, dcXR = 0.0f, dcYR = 0.0f;
    SmoothedValue sweepSm, depthSm, fbSm;
};

} // namespace moth
