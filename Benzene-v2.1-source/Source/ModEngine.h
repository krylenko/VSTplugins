#pragma once
#include <cmath>
#include <cstdint>

// ============================================================
//  Benzene modulation engine
//
//  Three modulation sources, each producing a bipolar value in -1..+1 that is
//  computed once per processing block and routed through a depth matrix to the
//  continuous controls of the signal generator and ring modulator.
//
//    1. LFOSource          — free-running, speed + tri/saw/square blend
//    2. PitchTrackerSource — zero-crossing pitch estimate of the input
//    3. EnvFollowerSource  — amplitude envelope of the input
// ============================================================

namespace bz
{

// ============================================================
//  LFOSource
// ============================================================
class LFOSource
{
public:
    void prepare (double sr) noexcept { sampleRate = sr; phase = 0.0; }

    void setRateHz (float hz) noexcept { rateHz = hz; }
    // blend: 0 = triangle, 0.5 = sawtooth, 1 = square (smooth morph)
    void setBlend (float b)   noexcept { blend = b; }

    // advance by 'samples' and return the current bipolar value
    float process (int samples) noexcept
    {
        phase += (double) rateHz * (double) samples / sampleRate;
        while (phase >= 1.0) phase -= 1.0;
        while (phase < 0.0)  phase += 1.0;

        float p   = (float) phase;
        float tri = 1.0f - 4.0f * std::fabs (p - 0.5f);  // +1 at 0, -1 at 0.5
        float saw = 2.0f * p - 1.0f;
        float sq  = (p < 0.5f) ? 1.0f : -1.0f;

        if (blend < 0.5f)
        {
            float t = blend / 0.5f;
            return tri * (1.0f - t) + saw * t;
        }
        else
        {
            float t = (blend - 0.5f) / 0.5f;
            // soften the square a touch so it isn't a hard digital edge
            float softSq = std::tanh (sq * 3.0f);
            return saw * (1.0f - t) + softSq * t;
        }
    }

    float current() const noexcept { return lastVal; }
    void  store (float v) noexcept { lastVal = v; }

private:
    double sampleRate = 44100.0;
    double phase = 0.0;
    float  rateHz = 1.0f;
    float  blend = 0.0f;
    float  lastVal = 0.0f;
};

// ============================================================
//  PitchTrackerSource — zero-crossing estimate
//
//  Cheap and robust enough for a modulation source. Estimates the input's
//  fundamental from the zero-crossing rate over the block, smooths it, and maps
//  it to a bipolar value relative to a reference so that "higher than reference"
//  pushes positive and "lower" pushes negative. Sensitivity scales the range.
// ============================================================
class PitchTrackerSource
{
public:
    void prepare (double sr) noexcept
    {
        sampleRate = sr;
        smoothHz = refHz;
        prevSign = false;
        lastVal = 0.0f;
    }

    void setSensitivity (float s) noexcept { sensitivity = s; }

    // process a block of input samples, return bipolar pitch value
    float process (const float* data, int n) noexcept
    {
        if (n <= 0) return lastVal;

        // count zero crossings (with a small hysteresis to ignore noise)
        int zc = 0;
        float hyst = 0.01f;
        bool sign = prevSign;
        for (int i = 0; i < n; ++i)
        {
            float x = data[i];
            if (sign && x < -hyst)      { sign = false; ++zc; }
            else if (! sign && x > hyst) { sign = true;  ++zc; }
        }
        prevSign = sign;

        // freq estimate: zc crossings = zc/2 cycles over n samples
        float estHz = (zc * 0.5f) * (float) sampleRate / (float) n;

        // only trust it if there was real signal energy and a sane range
        float rms = 0.0f;
        for (int i = 0; i < n; ++i) rms += data[i] * data[i];
        rms = std::sqrt (rms / (float) n);

        if (rms > 0.003f && estHz > 20.0f && estHz < 5000.0f)
        {
            // smooth across blocks to avoid jitter
            smoothHz += 0.25f * (estHz - smoothHz);
        }
        // (if no reliable pitch, hold the last smoothed value)

        // map to bipolar: log2 ratio vs reference, +/- 2 octaves full scale
        float ratio = smoothHz / refHz;
        if (ratio < 0.0001f) ratio = 0.0001f;
        float octaves = std::log2 (ratio);            // 0 at reference
        float v = (octaves / 2.0f) * sensitivity;     // 2 octaves = full at sens 1
        if (v >  1.0f) v =  1.0f;
        if (v < -1.0f) v = -1.0f;

        lastVal = v;
        return v;
    }

    float current() const noexcept { return lastVal; }

private:
    double sampleRate = 44100.0;
    float  sensitivity = 0.5f;
    float  smoothHz = 220.0f;
    bool   prevSign = false;
    float  lastVal = 0.0f;
    static constexpr float refHz = 220.0f;   // centre pitch (A3)
};

// ============================================================
//  EnvFollowerSource — amplitude envelope
//
//  Tracks the input level with adjustable speed, scaled by sensitivity, and
//  returned as a bipolar value (centred so 0 input -> -1, loud -> +1-ish) so it
//  behaves consistently with the bipolar depth matrix.
// ============================================================
class EnvFollowerSource
{
public:
    void prepare (double sr) noexcept
    {
        sampleRate = sr;
        env = 0.0f;
        lastVal = 0.0f;
        updateCoeffs();
    }

    void setSensitivity (float s) noexcept { sensitivity = s; }
    // speed 0 = slow (smooth), 1 = fast (snappy)
    void setSpeed (float s) noexcept { speed = s; updateCoeffs(); }

    float process (const float* data, int n) noexcept
    {
        if (n <= 0) return lastVal;

        for (int i = 0; i < n; ++i)
        {
            float ax = std::fabs (data[i]);
            if (ax > env) env = atk * env + (1.0f - atk) * ax;
            else          env = rel * env + (1.0f - rel) * ax;
        }

        // scale by sensitivity (sensitivity boosts gain into the envelope)
        float e = env * (0.5f + sensitivity * 4.0f);
        if (e > 1.0f) e = 1.0f;

        // bipolar: silence -> -1, full -> +1
        float v = e * 2.0f - 1.0f;
        lastVal = v;
        return v;
    }

    float current() const noexcept { return lastVal; }

private:
    void updateCoeffs() noexcept
    {
        // speed maps to time constants: slow ~ (atk 20ms, rel 200ms),
        // fast ~ (atk 1ms, rel 20ms)
        float atkMs = 20.0f - speed * 19.0f;   // 20..1 ms
        float relMs = 200.0f - speed * 180.0f; // 200..20 ms
        atk = std::exp (-1.0f / (float) (sampleRate * atkMs * 0.001));
        rel = std::exp (-1.0f / (float) (sampleRate * relMs * 0.001));
    }

    double sampleRate = 44100.0;
    float  sensitivity = 0.5f;
    float  speed = 0.5f;
    float  env = 0.0f;
    float  atk = 0.0f, rel = 0.0f;
    float  lastVal = 0.0f;
};

} // namespace bz
