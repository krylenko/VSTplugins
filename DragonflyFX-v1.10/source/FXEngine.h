#pragma once
#include <cmath>
#include <cstring>
#include <algorithm>

// ============================================================
//  Wavetables (256-point float tables)
// ============================================================
static constexpr int FX_PHASE_SZ = 256;

static const float wt_saw_f[FX_PHASE_SZ] = {
    -1.000f,-0.992f,-0.984f,-0.977f,-0.969f,-0.961f,-0.953f,-0.945f,
    -0.937f,-0.930f,-0.922f,-0.914f,-0.906f,-0.898f,-0.891f,-0.883f,
    -0.875f,-0.867f,-0.859f,-0.852f,-0.844f,-0.836f,-0.828f,-0.820f,
    -0.813f,-0.805f,-0.797f,-0.789f,-0.781f,-0.773f,-0.766f,-0.758f,
    -0.750f,-0.742f,-0.734f,-0.727f,-0.719f,-0.711f,-0.703f,-0.695f,
    -0.688f,-0.680f,-0.672f,-0.664f,-0.656f,-0.648f,-0.641f,-0.633f,
    -0.625f,-0.617f,-0.609f,-0.602f,-0.594f,-0.586f,-0.578f,-0.570f,
    -0.563f,-0.555f,-0.547f,-0.539f,-0.531f,-0.523f,-0.516f,-0.508f,
    -0.500f,-0.492f,-0.484f,-0.477f,-0.469f,-0.461f,-0.453f,-0.445f,
    -0.438f,-0.430f,-0.422f,-0.414f,-0.406f,-0.398f,-0.391f,-0.383f,
    -0.375f,-0.367f,-0.359f,-0.352f,-0.344f,-0.336f,-0.328f,-0.320f,
    -0.313f,-0.305f,-0.297f,-0.289f,-0.281f,-0.273f,-0.266f,-0.258f,
    -0.250f,-0.242f,-0.234f,-0.227f,-0.219f,-0.211f,-0.203f,-0.195f,
    -0.188f,-0.180f,-0.172f,-0.164f,-0.156f,-0.148f,-0.141f,-0.133f,
    -0.125f,-0.117f,-0.109f,-0.102f,-0.094f,-0.086f,-0.078f,-0.070f,
    -0.063f,-0.055f,-0.047f,-0.039f,-0.031f,-0.023f,-0.016f,-0.008f,
     0.000f, 0.008f, 0.016f, 0.023f, 0.031f, 0.039f, 0.047f, 0.055f,
     0.063f, 0.070f, 0.078f, 0.086f, 0.094f, 0.102f, 0.109f, 0.117f,
     0.125f, 0.133f, 0.141f, 0.148f, 0.156f, 0.164f, 0.172f, 0.180f,
     0.188f, 0.195f, 0.203f, 0.211f, 0.219f, 0.227f, 0.234f, 0.242f,
     0.250f, 0.258f, 0.266f, 0.273f, 0.281f, 0.289f, 0.297f, 0.305f,
     0.313f, 0.320f, 0.328f, 0.336f, 0.344f, 0.352f, 0.359f, 0.367f,
     0.375f, 0.383f, 0.391f, 0.398f, 0.406f, 0.414f, 0.422f, 0.430f,
     0.438f, 0.445f, 0.453f, 0.461f, 0.469f, 0.477f, 0.484f, 0.492f,
     0.500f, 0.508f, 0.516f, 0.523f, 0.531f, 0.539f, 0.547f, 0.555f,
     0.563f, 0.570f, 0.578f, 0.586f, 0.594f, 0.602f, 0.609f, 0.617f,
     0.625f, 0.633f, 0.641f, 0.648f, 0.656f, 0.664f, 0.672f, 0.680f,
     0.688f, 0.695f, 0.703f, 0.711f, 0.719f, 0.727f, 0.734f, 0.742f,
     0.750f, 0.758f, 0.766f, 0.773f, 0.781f, 0.789f, 0.797f, 0.805f,
     0.813f, 0.820f, 0.828f, 0.836f, 0.844f, 0.852f, 0.859f, 0.867f,
     0.875f, 0.883f, 0.891f, 0.898f, 0.906f, 0.914f, 0.922f, 0.930f,
     0.937f, 0.945f, 0.953f, 0.961f, 0.969f, 0.977f, 0.984f, 0.992f
};

static const float wt_tri_f[FX_PHASE_SZ] = {
    -1.000f,-0.984f,-0.969f,-0.953f,-0.938f,-0.922f,-0.906f,-0.891f,
    -0.875f,-0.859f,-0.844f,-0.828f,-0.813f,-0.797f,-0.781f,-0.766f,
    -0.750f,-0.734f,-0.719f,-0.703f,-0.688f,-0.672f,-0.656f,-0.641f,
    -0.625f,-0.609f,-0.594f,-0.578f,-0.563f,-0.547f,-0.531f,-0.516f,
    -0.500f,-0.484f,-0.469f,-0.453f,-0.438f,-0.422f,-0.406f,-0.391f,
    -0.375f,-0.359f,-0.344f,-0.328f,-0.313f,-0.297f,-0.281f,-0.266f,
    -0.250f,-0.234f,-0.219f,-0.203f,-0.188f,-0.172f,-0.156f,-0.141f,
    -0.125f,-0.109f,-0.094f,-0.078f,-0.063f,-0.047f,-0.031f,-0.016f,
     0.000f, 0.016f, 0.031f, 0.047f, 0.063f, 0.078f, 0.094f, 0.109f,
     0.125f, 0.141f, 0.156f, 0.172f, 0.188f, 0.203f, 0.219f, 0.234f,
     0.250f, 0.266f, 0.281f, 0.297f, 0.313f, 0.328f, 0.344f, 0.359f,
     0.375f, 0.391f, 0.406f, 0.422f, 0.438f, 0.453f, 0.469f, 0.484f,
     0.500f, 0.516f, 0.531f, 0.547f, 0.563f, 0.578f, 0.594f, 0.609f,
     0.625f, 0.641f, 0.656f, 0.672f, 0.688f, 0.703f, 0.719f, 0.734f,
     0.750f, 0.766f, 0.781f, 0.797f, 0.813f, 0.828f, 0.844f, 0.859f,
     0.875f, 0.891f, 0.906f, 0.922f, 0.938f, 0.953f, 0.969f, 0.984f,
     1.000f, 0.984f, 0.969f, 0.953f, 0.938f, 0.922f, 0.906f, 0.891f,
     0.875f, 0.859f, 0.844f, 0.828f, 0.813f, 0.797f, 0.781f, 0.766f,
     0.750f, 0.734f, 0.719f, 0.703f, 0.688f, 0.672f, 0.656f, 0.641f,
     0.625f, 0.609f, 0.594f, 0.578f, 0.563f, 0.547f, 0.531f, 0.516f,
     0.500f, 0.484f, 0.469f, 0.453f, 0.438f, 0.422f, 0.406f, 0.391f,
     0.375f, 0.359f, 0.344f, 0.328f, 0.313f, 0.297f, 0.281f, 0.266f,
     0.250f, 0.234f, 0.219f, 0.203f, 0.188f, 0.172f, 0.156f, 0.141f,
     0.125f, 0.109f, 0.094f, 0.078f, 0.063f, 0.047f, 0.031f, 0.016f,
     0.000f,-0.016f,-0.031f,-0.047f,-0.063f,-0.078f,-0.094f,-0.109f,
    -0.125f,-0.141f,-0.156f,-0.172f,-0.188f,-0.203f,-0.219f,-0.234f,
    -0.250f,-0.266f,-0.281f,-0.297f,-0.313f,-0.328f,-0.344f,-0.359f,
    -0.375f,-0.391f,-0.406f,-0.422f,-0.438f,-0.453f,-0.469f,-0.484f,
    -0.500f,-0.516f,-0.531f,-0.547f,-0.563f,-0.578f,-0.594f,-0.609f,
    -0.625f,-0.641f,-0.656f,-0.672f,-0.688f,-0.703f,-0.719f,-0.734f,
    -0.750f,-0.766f,-0.781f,-0.797f,-0.813f,-0.828f,-0.844f,-0.859f,
    -0.875f,-0.891f,-0.906f,-0.922f,-0.938f,-0.953f,-0.969f,-0.984f
};

static const float wt_squ_f[FX_PHASE_SZ] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const float wt_noise_f[FX_PHASE_SZ] = {
     0.969f,-0.922f, 0.766f, 0.820f, 0.586f,-0.805f,-0.477f,-0.328f,
     0.352f,-0.727f, 0.438f,-0.789f, 0.305f,-0.016f, 0.555f, 0.422f,
     0.797f, 0.773f,-0.336f, 0.391f,-0.609f,-0.938f, 0.484f, 0.000f,
    -0.047f, 0.805f, 0.219f, 0.234f, 0.711f, 0.602f, 0.148f,-0.633f,
    -0.523f, 0.766f,-0.945f,-0.023f,-0.664f, 0.953f, 0.422f, 0.000f,
    -0.063f,-0.883f, 0.359f,-0.914f,-0.859f, 0.039f,-0.805f, 0.633f,
     0.625f, 0.438f,-0.703f, 0.313f, 0.031f, 0.938f, 0.289f, 0.594f,
    -0.094f,-0.141f, 0.641f,-0.836f,-0.742f,-0.656f,-0.219f, 0.656f,
     0.602f,-0.883f,-0.203f, 0.047f,-0.172f, 0.305f, 0.250f,-0.422f,
    -0.141f,-0.969f, 0.961f,-0.664f,-0.789f,-0.258f,-0.602f,-0.023f,
    -0.320f, 0.898f, 0.836f,-0.898f, 0.469f,-0.461f,-0.156f, 0.094f,
     0.875f,-0.164f, 0.961f,-0.398f, 0.398f, 0.328f, 0.070f, 0.391f,
     0.328f,-0.648f,-0.742f, 1.000f,-0.656f,-0.938f, 0.117f, 0.758f,
     0.336f,-0.617f,-0.266f,-0.086f, 0.953f,-0.688f, 0.703f, 0.281f,
    -0.250f,-0.617f,-0.148f,-0.039f,-0.758f, 0.172f,-0.547f,-0.234f,
     0.164f,-0.500f,-0.422f, 0.227f,-0.469f, 0.641f, 0.961f, 0.453f,
    -0.313f, 0.164f,-0.789f, 0.805f, 0.750f, 0.633f,-0.484f, 0.188f,
    -0.953f,-0.156f,-0.375f,-0.680f,-0.641f,-0.156f,-0.813f, 0.195f,
    -0.063f, 0.383f, 0.391f, 0.273f,-0.930f,-0.859f,-0.367f, 0.055f,
     0.305f,-0.188f, 0.633f, 0.430f, 0.930f, 0.055f,-0.352f,-0.789f,
     0.219f, 0.555f,-0.156f,-0.820f,-0.469f,-0.695f,-0.438f,-0.125f,
     0.047f,-0.086f, 0.742f, 0.031f, 0.883f, 0.273f, 0.906f,-0.523f,
     0.344f,-0.422f, 0.336f, 0.383f,-0.867f,-0.492f,-0.555f, 0.328f,
     0.680f,-0.313f, 0.555f, 0.344f,-0.984f, 0.203f,-0.227f, 0.828f,
    -1.000f,-0.078f,-0.156f,-0.078f, 0.531f,-0.359f, 0.563f,-0.063f,
    -0.930f,-0.648f, 0.438f,-0.055f,-0.695f,-0.320f, 0.211f,-0.617f,
     0.469f,-0.516f, 0.828f,-0.461f, 0.523f,-0.625f,-0.430f,-0.820f,
     0.148f, 0.359f, 0.086f,-0.148f, 0.281f, 0.289f, 0.352f, 0.266f,
     0.883f,-0.586f, 0.414f,-0.531f,-0.766f, 0.211f,-0.102f,-0.086f,
     0.320f, 0.531f,-0.305f, 0.320f,-0.172f, 0.680f, 0.656f,-0.492f,
     0.219f, 0.156f, 0.078f, 0.734f,-0.469f,-0.367f,-0.766f, 0.875f,
     0.289f,-0.047f, 0.273f, 0.086f, 0.289f, 0.086f, 0.438f, 0.039f
};

// ============================================================
//  Enums
// ============================================================
enum FXType  { FX_BYPASS = 0, FX_MS20_LP = 1, FX_MS20_HP = 2, FX_COMB = 3, FX_CHEBY = 4 };
enum LFOWave { LFO_SAW = 0, LFO_TRI = 1, LFO_NOISE = 2, LFO_SQUARE = 3 };

enum LFOTarget {
    TARG_B0_PARAM0 = 0, TARG_B0_PARAM1,
    TARG_B1_PARAM0,     TARG_B1_PARAM1,
    TARG_B2_PARAM0,     TARG_B2_PARAM1,
    TARG_B3_PARAM0,     TARG_B3_PARAM1,
    TARG_NONE = 8,
    TARG_COUNT = 9
};

// ============================================================
//  DFSmoothedValue — one-pole parameter smoother (click-free
//  changes to things like the global wet/dry mix).
// ============================================================
struct DFSmoothedValue
{
    float current = 0.0f;
    float target  = 0.0f;
    float coeff   = 0.0f;   // per-sample

    void setTimeConstant(float ms, double sr)
    {
        float samples = (float)(ms * 0.001 * sr);
        if (samples < 1.0f) samples = 1.0f;
        coeff = std::exp(-1.0f / samples);
    }
    void setImmediate(float v) { current = target = v; }
    inline float next()
    {
        current = target + (current - target) * coeff;
        return current;
    }
};

// ============================================================
//  MS-20 style nonlinear TPT filter  (see processMS20 for full design notes)
// ============================================================
struct FXBlock
{
    FXType type    = FX_BYPASS;
    float  param0  = 1.0f;   // cutoff (normalised 0..1)
    float  param1  = 0.0f;   // resonance 0..1

    float eff_param0 = 1.0f;
    float eff_param1 = 0.0f;

    float smooth_p0 = 1.0f;
    float smooth_p1 = 0.0f;

    // Two integrator states
    float s1 = 0.0f, s2 = 0.0f;

    // Comb buffer
    static constexpr int COMB_MAX = 96000;
    float combBuf[COMB_MAX] = {};
    int   combWrite = 0;

    double sampleRate = 44100.0;

    // ---- DC blocker (automatic, v1.10) ----
    // Applied automatically to the two block types that generate DC:
    //  * Chebyshev: even orders (T2/T4/T6, and blends involving them) are
    //    even functions, so they both carry a constant term (T_n(0) = ±1,
    //    present even in silence) and rectify real audio into a bias.
    //  * MS-20 LP: the asymmetric JFET clip creates even harmonics and a
    //    small resonance-dependent DC offset.
    // MS-20 HP (DC removed before a symmetric tanh) and Comb (linear) don't
    // generate DC, so they're left untouched. Blocking at the source also
    // protects a downstream Comb, whose feedback would amplify incoming DC.
    // One-pole HPF at ~8 Hz: inaudible on the harmonic content itself.
    float dcX = 0.0f, dcY = 0.0f, dcCoeff = 0.0f;

    static bool typeNeedsDCBlock(FXType t) { return t == FX_CHEBY || t == FX_MS20_LP; }

    void reset()
    {
        s1 = s2 = 0.0f;
        std::fill(combBuf, combBuf + COMB_MAX, 0.0f);
        combWrite = 0;
        smooth_p0 = param0;
        smooth_p1 = param1;
        dcX = dcY = 0.0f;
        dcCoeff = std::exp(-2.0f * 3.14159265f * 8.0f / (float)sampleRate);
    }

    float process(float in)
    {
        if (type == FX_BYPASS) return in;

        const float sr = (float)sampleRate;
        // ~30Hz smoother: fast enough for LFO, slow enough to kill zipper noise
        const float sc = std::exp(-2.0f * 3.14159265f * 30.0f / sr);
        smooth_p0 = sc * smooth_p0 + (1.0f - sc) * std::max(0.0f, std::min(1.0f, eff_param0));
        smooth_p1 = sc * smooth_p1 + (1.0f - sc) * std::max(0.0f, std::min(1.0f, eff_param1));

        float out;
        if (type == FX_MS20_LP || type == FX_MS20_HP)
            out = processMS20(in);
        else if (type == FX_CHEBY)
            out = processCheby(in);
        else
            out = processComb(in);

        // The blocker runs continuously for every active block so its state
        // is always current: switching a block to Chebyshev/LP engages it
        // without a step, since it's already tracking the signal.
        const float blocked = blockDC(out);
        return typeNeedsDCBlock(type) ? blocked : out;
    }

private:
    inline float blockDC(float x)
    {
        // y[n] = x[n] - x[n-1] + R*y[n-1]
        float y = x - dcX + dcCoeff * dcY;
        dcX = x;
        dcY = y;
        return y;
    }

    // ---------------------------------------------------------------
    //  Inline saturation functions
    // ---------------------------------------------------------------

    // Asymmetric JFET-style soft clip -- used on LP output for tone colour.
    static inline float jfetClip(float x)
    {
        if (x >= 0.0f)
            return x / (1.0f + x * 0.5f);
        else
            return x / (1.0f - x * 0.3f);
    }

    // ---------------------------------------------------------------
    //  MS-20 nonlinear TPT SVF  (v4 — correct topology)
    //
    //  ROOT CAUSE (v1-v3): used a SEQUENTIAL cascade topology but applied
    //  the Zavalishin SVF HP identity (HP = v1_in - v1 - v2) to it.
    //  That identity only holds for the SIMULTANEOUS SVF solve.
    //  In the sequential cascade: at DC, v1→in, v2→tanh(in), so
    //  "HP" = in - in - tanh(in) = -tanh(in) ≠ 0.  Not a high-pass at all.
    //
    //  FIX (v4): use the correct Zavalishin TPT SVF with simultaneous
    //  implicit solve. HP, BP, LP are all derived from the same solve and
    //  satisfy HP + 2R*BP + LP = input exactly.
    //
    //  Nonlinearity: tanh on STATE UPDATE (not on integrator outputs).
    //  This bounds self-oscillation without breaking the linear solve
    //  that produces correct HP/LP at all resonance settings. The saturated
    //  states feed back into next sample's solve, adding harmonic richness.
    //
    //  Resonance: twoR = 2*(1-r)^2 gives a squared curve:
    //    r=0.0  Q=0.5    no resonance, Butterworth-like
    //    r=0.3  Q=1.0    clearly audible peak
    //    r=0.5  Q=2.0    strong resonance
    //    r=0.7  Q=5.6    aggressive
    //    r=0.9  Q=50     near self-oscillation
    //    r=1.0  Q=inf    self-oscillation, bounded by tanh on states
    // ---------------------------------------------------------------
    float processMS20(float in)
    {
        const float sr = (float)sampleRate;

        float cutHz = 20.0f * std::pow(1000.0f, smooth_p0);
        cutHz = std::max(20.0f, std::min(cutHz, sr * 0.48f));

        float g = std::tan(3.14159265f * cutHz / sr);

        float r = smooth_p1;

        // Damping: squared (1-r) curve for early resonance onset.
        // twoR=2 at r=0 (Q=0.5), twoR=0 at r=1 (self-oscillation).
        float omr  = 1.0f - r;
        float twoR = 2.0f * omr * omr;

        // ---- Zavalishin TPT SVF: simultaneous implicit solve ----
        // hp = (in - (2R+g)*s1 - s2) / (1 + 2R*g + g*g)
        // bp = g*hp + s1
        // lp = g*bp + s2
        // Identity: hp + 2R*bp + lp = in  (always, by construction)
        float denom = 1.0f / (1.0f + twoR * g + g * g);
        float hp = (in - (twoR + g) * s1 - s2) * denom;
        float bp = g * hp + s1;
        float lp = g * bp + s2;

        // ---- State update: tanh saturation ----
        // Bounds self-oscillation amplitude at twoR=0 and adds harmonic
        // content to the feedback loop (states feed into next sample's solve).
        // At low resonance the states are small, tanh is ~linear, filter is clean.
        s1 = std::tanh(2.0f * bp - s1);
        s2 = std::tanh(2.0f * lp - s2);

        if (type == FX_MS20_LP)
        {
            // LP output: JFET asymmetric clip for nasal MS-20 character.
            // Drive scales with resonance so the resonant peak hits the
            // saturation harder, producing more harmonics as you turn it up.
            float drive = 1.0f + r * 3.0f;
            return std::max(-1.0f, std::min(1.0f, jfetClip(lp * drive)));
        }
        else
        {
            // HP output: tanh saturation with resonance-scaled drive.
            float drive = 1.0f + r * 2.5f;
            return std::max(-1.0f, std::min(1.0f, std::tanh(hp * drive)));
        }
    }

    // ---------------------------------------------------------------
    //  Comb filter (unchanged)
    // ---------------------------------------------------------------
    float processComb(float in)
    {
        const float sr = (float)sampleRate;
        float freqHz   = 20.0f * std::pow(1000.0f, smooth_p0);
        float delayLen = sr / std::max(1.0f, freqHz);
        int   delaySmp = (int)std::min(delayLen, (float)(COMB_MAX - 1));
        int   readPos  = (combWrite - delaySmp + COMB_MAX) % COMB_MAX;
        float delayed  = combBuf[readPos];
        float fb       = smooth_p1 * 0.99f;
        float out      = in + fb * delayed;
        combBuf[combWrite] = out;
        combWrite = (combWrite + 1) % COMB_MAX;
        return std::max(-1.0f, std::min(1.0f, out));
    }

    // ---------------------------------------------------------------
    //  Chebyshev polynomial waveshaper
    //
    //  Chebyshev polynomials of the first kind have the property that
    //  T_k(cos(θ)) = cos(k·θ), so applying T_k to a sine wave produces
    //  exactly the kth harmonic.  Mixing polynomials of different orders
    //  creates controlled harmonic spectra.
    //
    //  Controls:
    //    param0 = Drive (0..1 → 0.5x..10x input gain)
    //    param1 = Order blend (0..1 → continuous morph T2 through T6)
    //             0.00 = T2 (octave up / frequency doubling)
    //             0.25 = T3 (3rd harmonic, nasal/hollow)
    //             0.50 = T4 (4th harmonic, bright)
    //             0.75 = T5 (5th harmonic, aggressive)
    //             1.00 = T6 (6th harmonic, metallic)
    //             Values between snap points blend two adjacent orders.
    // ---------------------------------------------------------------

    // Evaluate Chebyshev polynomial T_n(x) for n=1..5 using the
    // recurrence relation T_{k+1}(x) = 2x·T_k(x) - T_{k-1}(x)
    static inline float chebyT(float x, int n)
    {
        if (n <= 0) return 1.0f;       // T0
        if (n == 1) return x;           // T1
        float t_prev2 = 1.0f;           // T0
        float t_prev1 = x;              // T1
        for (int k = 2; k <= n; ++k) {
            float t = 2.0f * x * t_prev1 - t_prev2;
            t_prev2 = t_prev1;
            t_prev1 = t;
        }
        return t_prev1;
    }

    float processCheby(float in)
    {
        // Drive: 0..1 → 0.5x..10x input gain.
        // Low drive = subtle harmonic colouring, high drive = heavy distortion.
        float drive = 0.5f + smooth_p0 * 9.5f;
        float x = in * drive;

        // Order blend: 0..1 → continuous morph from T2 to T6.
        float orderF = 2.0f + smooth_p1 * 4.0f;  // 2.0 .. 6.0
        int orderLo   = (int)orderF;
        int orderHi   = orderLo + 1;
        float frac    = orderF - (float)orderLo;
        if (orderHi > 6) { orderHi = 6; frac = 0.0f; }

        // Evaluate both polynomials and blend
        float yLo = chebyT(x, orderLo);
        float yHi = chebyT(x, orderHi);
        float y   = yLo + frac * (yHi - yLo);

        // Output soft-clip: tanh keeps the output musical when driven hard.
        // For low drive with |x| ≤ 1, the polynomial output is already in [-1,1]
        // so tanh is near-transparent.
        return std::tanh(y);
    }
};

// ============================================================
//  LFOUnit
// ============================================================
struct LFOUnit
{
    LFOWave  wave       = LFO_TRI;
    float    speedHz    = 1.0f;
    float    phase      = 0.0f;
    double   sampleRate = 44100.0;
    float    value      = 0.0f;

    void tick()
    {
        const float* wt = getTable();
        value = wt[(int)phase & (FX_PHASE_SZ - 1)];
        float inc = speedHz * ((float)FX_PHASE_SZ / (float)sampleRate);
        phase += inc;
        if (phase >= (float)FX_PHASE_SZ) phase -= (float)FX_PHASE_SZ;
    }

private:
    const float* getTable() const
    {
        switch (wave) {
            case LFO_SAW:   return wt_saw_f;
            case LFO_TRI:   return wt_tri_f;
            case LFO_NOISE: return wt_noise_f;
            default:        return wt_squ_f;
        }
    }
};
// ============================================================
//  Envelope Follower — tracks input amplitude for modulation
//
//  Rectifies the input, applies sensitivity gain, then smooths
//  with an asymmetric one-pole filter (attack 4x faster than
//  release for punchy transient tracking).
//
//  Output: 0..1 unipolar envelope value, updated per sample.
// ============================================================
struct EnvFollower
{
    float sensitivity = 0.5f;   // 0..1 → 1x..20x input gain
    float speed       = 0.5f;   // 0..1 → 1ms..500ms smoothing
    float envelope    = 0.0f;   // current output 0..1
    double sampleRate = 44100.0;

    void tick(float input)
    {
        // Rectify and apply sensitivity gain
        float sensGain  = 1.0f + sensitivity * 19.0f;
        float rectified = std::abs(input) * sensGain;

        // Smoothing time constant: 1ms (fast) to 500ms (slow)
        float timeMs     = 1.0f + speed * 499.0f;
        float releaseCoeff = std::exp(-1000.0f / (timeMs * (float)sampleRate));
        // Attack is 4x faster than release for punchy transient tracking
        float attackCoeff  = std::exp(-4000.0f / (timeMs * (float)sampleRate));

        float coeff = (rectified > envelope) ? attackCoeff : releaseCoeff;
        envelope = coeff * envelope + (1.0f - coeff) * rectified;

        // Clamp to 0..1
        envelope = std::min(1.0f, std::max(0.0f, envelope));
    }
};
