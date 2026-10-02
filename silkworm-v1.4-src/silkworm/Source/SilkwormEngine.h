#pragma once
#include <cstdint>
#include <cmath>
#include <cstring>
#include <algorithm>

// ============================================================
//  Silkworm Reverb Engine
//
//  Allpass-loop reverb inspired by Keith Barr's topology
//  (Alesis Midiverb / Spin FV-1 lineage).
//
//  Signal path:
//    Mono sum → Predelay → 4× input diffusion allpass →
//    Recirculating loop of 4 stages (allpass + delay + lowpass) →
//    Stereo output taps via Hadamard-like matrix
//
//  Lo-fi character via:
//    - Variable bit-depth quantisation in the feedback path
//    - One-pole lowpass darkening per stage
//    - Delay-time LFO modulation (chorus/swirl)
//    - Soft saturation (tanh)
//    - Clock-speed decimator (sample-and-hold)
// ============================================================

static constexpr float SW_PI = 3.14159265358979323846f;

// Buffer sizes (power-of-2 for fast masking)
static constexpr int PDLY_SIZE  = 65536;   // predelay: covers 500 ms @ 96 kHz
static constexpr int PDLY_MASK  = PDLY_SIZE - 1;
static constexpr int IAP_SIZE   = 2048;    // input allpass buffers
static constexpr int IAP_MASK   = IAP_SIZE - 1;
static constexpr int LAP_SIZE   = 8192;    // loop allpass buffers
static constexpr int LAP_MASK   = LAP_SIZE - 1;
static constexpr int LDL_SIZE   = 8192;    // loop delay buffers
static constexpr int LDL_MASK   = LDL_SIZE - 1;

// Number of input-diffusion and loop stages
static constexpr int NUM_INPUT_AP = 4;
static constexpr int NUM_LOOP     = 4;

// Base delay times in samples at 44 100 Hz (all prime or coprime)
//   Input diffusion allpasses — short, for smearing the impulse
static constexpr int BASE_IAP_DELAY[NUM_INPUT_AP] = { 211, 281, 367, 499 };
//   Loop allpasses — medium, for recirculating density
static constexpr int BASE_LAP_DELAY[NUM_LOOP]     = { 601, 797, 1061, 1409 };
//   Loop plain delays — interleaved with allpasses
static constexpr int BASE_LDL_DELAY[NUM_LOOP]     = { 887, 1171, 1553, 2063 };

// LFO phase offsets per loop stage (radians) — avoids correlated modulation
static constexpr float LFO_OFFSET[NUM_LOOP] = { 0.0f, 1.571f, 3.142f, 4.712f };

// ============================================================
class SilkwormEngine
{
public:
    struct Params {
        float  predelayMs    = 0.0f;     // 0 – 500
        float  density       = 0.5f;     // 0 – 1
        float  character     = 0.0f;     // 0 – 1
        float  cutoffOffset  = 0.0f;     // -1 – +1 bipolar LP cutoff shift
        float  modAmount     = 0.0f;     // 0 – 1 additional modulation depth
        float  decaySec      = 2.0f;     // 0.1 – 45
        float  clockSpeed    = 1.0f;     // 0.25 – 1.0
        float  mix           = 0.5f;     // 0 – 1  (dry/wet)
        float  volume        = 0.7f;     // 0 – 1
        double sampleRate    = 44100.0;
    };

    // Call once from prepareToPlay
    void prepare(double sampleRate)
    {
        sr = (float)sampleRate;

        // Scale all delay lengths from base (44 100 Hz) to actual SR
        float ratio = sr / 44100.0f;
        for (int i = 0; i < NUM_INPUT_AP; ++i)
            iapLen[i] = std::max(1, (int)std::round(BASE_IAP_DELAY[i] * ratio));
        for (int i = 0; i < NUM_LOOP; ++i) {
            lapLen[i] = std::max(1, (int)std::round(BASE_LAP_DELAY[i] * ratio));
            ldlLen[i] = std::max(1, (int)std::round(BASE_LDL_DELAY[i] * ratio));
        }

        // Compute total loop time (used for feedback gain calculation)
        loopSamples = 0;
        for (int i = 0; i < NUM_LOOP; ++i)
            loopSamples += lapLen[i] + ldlLen[i];

        // Clear all buffers
        reset();
    }

    void reset()
    {
        std::memset(predelayBuf, 0, sizeof(predelayBuf));
        pdlyW = 0;
        for (int i = 0; i < NUM_INPUT_AP; ++i) {
            std::memset(iapBuf[i], 0, sizeof(iapBuf[i]));
            iapW[i] = 0;
        }
        for (int i = 0; i < NUM_LOOP; ++i) {
            std::memset(lapBuf[i], 0, sizeof(lapBuf[i]));
            std::memset(ldlBuf[i], 0, sizeof(ldlBuf[i]));
            lapW[i] = 0;
            ldlW[i] = 0;
            lpState[i] = 0.0f;
        }
        lfoPhase  = 0.0f;
        dcX = dcY = 0.0f;
        inputDampState = 0.0f;
        clockPhase = 0.0f;
        holdL = holdR = 0.0f;
        loopFeedback = 0.0f;
    }

    // Process a stereo buffer in-place
    void process(const float* inL, const float* inR,
                 float* outL, float* outR, int numSamples,
                 const Params& p)
    {
        // Derive internal coefficients from params
        // Density: wide range for dramatic effect
        //   Input diffusion: 0 (pass-through) → 0.75 (heavy smearing)
        //   Loop allpass:    0.1 (sparse/metallic) → 0.85 (dense wash)
        const float inputApCoeff = p.density * 0.75f;           // 0.0 – 0.75
        const float loopApCoeff  = 0.1f + p.density * 0.75f;    // 0.1 – 0.85
        const float lpCutHz     = 18000.0f * std::pow(1500.0f / 18000.0f,
                                    std::fmax(0.0f, std::fmin(2.0f,
                                        p.character - p.cutoffOffset)));
        const float lpCoeff     = std::exp(-2.0f * SW_PI * lpCutHz / sr);
        const float crushBits   = 16.0f - p.character * 8.0f; // 16 → 8
        const float crushLevels = std::pow(2.0f, crushBits);
        const float modDepth    = 0.4f                        // always-on baseline: smears
                                + p.character * 3.0f         // static resonant peaks
                                + p.modAmount * 30.0f;       // +0 → +30 samples
        const float satDrive    = 1.0f + p.character * 2.0f;   // 1 → 3
        // Energy-safe saturation: tanh(x*d)/d has derivative sech²(dx) ≤ 1
        // everywhere, so it can never inject energy into the feedback loop.
        // (The old tanh(x*d)/tanh(d) had gain d/tanh(d) ≈ 3x at small
        // amplitudes with drive=3, which was the oscillation source.)
        const float satInvDrive = 1.0f / satDrive;

        const float loopTimeSec = (float)loopSamples / sr;
        const float fbGain      = std::min(0.985f,
                                  std::pow(10.0f, -3.0f * loopTimeSec
                                           / std::max(0.05f, p.decaySec)));

        const int   pdlySamples = std::min((int)(p.predelayMs * 0.001f * sr),
                                           PDLY_SIZE - 1);

        // LFO: base rate + additional speed at high mod settings
        const float lfoHz  = 0.6f + p.modAmount * 1.9f;  // 0.6 → 2.5 Hz
        const float lfoInc = lfoHz / sr;

        // Input damping: gentle LP at ~12 kHz to limit the sharpest
        // HF entering the loop, reducing metallic buildup in long tails
        // without noticeably darkening the initial sound.
        const float idampHz    = 12000.0f;
        const float idampCoeff = std::exp(-2.0f * SW_PI * idampHz / sr);

        const float wet = p.mix * p.volume;
        const float dry = (1.0f - p.mix) * p.volume;

        for (int n = 0; n < numSamples; ++n)
        {
            float dryL = inL[n];
            float dryR = inR[n];

            // Clock-speed decimator: process reverb at reduced rate
            clockPhase += p.clockSpeed;
            if (clockPhase >= 1.0f)
            {
                clockPhase -= 1.0f;

                // ---- Mono input ----
                float mono = (dryL + dryR) * 0.5f;

                // ---- Predelay ----
                predelayBuf[pdlyW & PDLY_MASK] = mono;
                float pd = predelayBuf[(pdlyW - pdlySamples) & PDLY_MASK];
                ++pdlyW;

                // ---- Input diffusion (4 series allpass) ----
                float sig = pd;
                for (int i = 0; i < NUM_INPUT_AP; ++i)
                    sig = processAllpass(sig, iapBuf[i], IAP_MASK,
                                         iapLen[i], iapW[i], inputApCoeff);

                // ---- Input damping: gentle LP at ~12 kHz ----
                // Limits the sharpest HF entering the loop without
                // noticeably darkening the initial sound.
                inputDampState = idampCoeff * inputDampState
                               + (1.0f - idampCoeff) * sig;
                sig = inputDampState;

                // ---- Inject into loop (add feedback from previous circulation) ----
                sig += loopFeedback;

                // ---- 4-stage recirculating loop ----
                // Each stage: modulated allpass → plain delay → one-pole LP
                float stageOut[NUM_LOOP];
                for (int i = 0; i < NUM_LOOP; ++i)
                {
                    // Modulated allpass
                    float mod = modDepth * std::sin(
                        2.0f * SW_PI * (lfoPhase + LFO_OFFSET[i] / (2.0f * SW_PI)));
                    sig = processModAllpass(sig, lapBuf[i], LAP_MASK,
                                            lapLen[i], lapW[i], loopApCoeff, mod);

                    // Plain delay
                    ldlBuf[i][ldlW[i] & LDL_MASK] = sig;
                    sig = ldlBuf[i][(ldlW[i] - ldlLen[i]) & LDL_MASK];
                    ++ldlW[i];

                    // One-pole lowpass in feedback path
                    lpState[i] = lpCoeff * lpState[i] + (1.0f - lpCoeff) * sig;
                    sig = lpState[i];

                    stageOut[i] = sig;
                }

                // ---- Feedback processing (lo-fi character) ----
                // Bit-crush
                sig = std::round(sig * crushLevels) / crushLevels;

                // Energy-safe saturation: tanh(x*d)/d
                // Derivative = sech²(x*d) ≤ 1, so never amplifies.
                // At character=0 (drive=1): tanh(x)/1 ≈ x, nearly transparent.
                // At character=1 (drive=3): aggressive waveshaping, bounded to ±1/3.
                sig = std::tanh(sig * satDrive) * satInvDrive;

                // DC blocker: y = x - x_prev + R * y_prev
                float dcR = 0.995f;
                float dcOut = sig - dcX + dcR * dcY;
                dcX = sig;
                dcY = dcOut;
                sig = dcOut;

                // Hard clip safety net — prevents runaway under any conditions
                sig = std::fmax(-1.0f, std::fmin(1.0f, sig));

                // Apply loop feedback gain
                loopFeedback = sig * fbGain;

                // ---- LFO advance ----
                lfoPhase += lfoInc;
                if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

                // ---- Stereo output taps (Hadamard-like rows) ----
                // L = s0 - s1 + s2 - s3   (row [+ - + -])
                // R = s0 + s1 - s2 - s3   (row [+ + - -])
                holdL = 0.35f * ( stageOut[0] - stageOut[1]
                                + stageOut[2] - stageOut[3]);
                holdR = 0.35f * ( stageOut[0] + stageOut[1]
                                - stageOut[2] - stageOut[3]);
            }

            // ---- Mix and output ----
            outL[n] = dry * dryL + wet * holdL;
            outR[n] = dry * dryR + wet * holdR;
        }
    }

private:
    float sr = 44100.0f;

    // Scaled delay lengths
    int iapLen[NUM_INPUT_AP] = {};
    int lapLen[NUM_LOOP]     = {};
    int ldlLen[NUM_LOOP]     = {};
    int loopSamples          = 0;      // total loop length for RT60 calc

    // Predelay
    float predelayBuf[PDLY_SIZE] = {};
    int   pdlyW = 0;

    // Input diffusion allpasses
    float iapBuf[NUM_INPUT_AP][IAP_SIZE] = {};
    int   iapW[NUM_INPUT_AP]             = {};

    // Loop allpasses
    float lapBuf[NUM_LOOP][LAP_SIZE] = {};
    int   lapW[NUM_LOOP]             = {};

    // Loop plain delays
    float ldlBuf[NUM_LOOP][LDL_SIZE] = {};
    int   ldlW[NUM_LOOP]             = {};

    // Per-stage lowpass state
    float lpState[NUM_LOOP] = {};

    // Input damping filter state
    float inputDampState = 0.0f;

    // LFO
    float lfoPhase = 0.0f;

    // DC blocker state
    float dcX = 0.0f, dcY = 0.0f;

    // Clock decimator
    float clockPhase = 0.0f;
    float holdL = 0.0f, holdR = 0.0f;

    // Recirculating feedback value
    float loopFeedback = 0.0f;

    // ----------------------------------------------------------
    //  Schroeder allpass:  H(z) = (-g + z^-D) / (1 - g z^-D)
    //
    //  v[n] = x[n] + g · v[n-D]
    //  y[n] = v[n-D] - g · v[n]
    // ----------------------------------------------------------
    static float processAllpass(float input, float* buf, int mask,
                                int delay, int& writePos, float g)
    {
        int readPos = (writePos - delay) & mask;
        float vD = buf[readPos];
        float v  = input + g * vD;
        buf[writePos & mask] = v;
        ++writePos;
        return vD - g * v;
    }

    // Modulated allpass — delay read position shifted by `modSamples`
    // Uses linear interpolation for fractional read offset
    static float processModAllpass(float input, float* buf, int mask,
                                   int delay, int& writePos, float g,
                                   float modSamples)
    {
        float readF = (float)writePos - (float)delay + modSamples;
        // Wrap into positive range
        while (readF < 0.0f) readF += (float)(mask + 1);
        int   idx0 = ((int)readF) & mask;
        int   idx1 = (idx0 + 1) & mask;
        float frac = readF - std::floor(readF);
        float vD   = buf[idx0] + frac * (buf[idx1] - buf[idx0]);

        float v = input + g * vD;
        buf[writePos & mask] = v;
        ++writePos;
        return vD - g * v;
    }
};
