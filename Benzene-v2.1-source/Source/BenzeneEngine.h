#pragma once
#include <cmath>
#include <cstdint>

static constexpr double BZ_TWO_PI = 6.283185307179586;

// ============================================================
//  Benzene — 1950s-style ring modulator
//
//  Two parts:
//    1. CarrierOscillator — a tube-style signal generator (sine/square),
//       0.01 Hz .. 100 kHz, with a soft triode-like nonlinearity and a
//       slow random drift so the carrier is never perfectly stable.
//    2. DiodeRing — a STATELESS waveshaper model (after Parker, DAFx-11).
//       Instead of solving the circuit ODEs, the diode pairs are modelled as
//       two static shaping functions fed by (carrier + input) and
//       (carrier - input). Their difference is the ring-modulated output.
//       Unconditionally stable (no state to diverge), a handful of ops per
//       sample, and it still produces the diode harmonics and grit. Light
//       oversampling tames aliasing cheaply because the shaper is memoryless.
//       Asymmetry / threshold mismatch give the raw character and a touch of
//       controllable carrier bleed.
// ============================================================

namespace bz
{

// ============================================================
//  CarrierOscillator — tube-style generator
// ============================================================
class CarrierOscillator
{
public:
    enum Shape { Sine = 0, Triangle = 1, Sawtooth = 2, Pulse = 3,
                 White = 4, Pink = 5, LFSR = 6 };

    void prepare (double sr) noexcept
    {
        sampleRate = sr;
        phase = 0.0;
        driftState = 0.0f;
        driftTarget = 0.0f;
        driftCount = 0;
        for (int i = 0; i < 7; ++i) pinkB[i] = 0.0f;
        noiseLP = 0.0f;
        lfsrReg = 0xACE1u;
        lfsrPhase = 0.0;
        lfsrHeld = 0.0f;
        srHold = 0.0f;
        srPhase = 0.0;
        dcX1 = dcY1 = 0.0f;
    }

    void setFrequency (float hz) noexcept { freq = hz; }
    void setShape (Shape s)     noexcept { shape = s; }
    void setDrive (float d)     noexcept { drive = d; }
    // Drift is context-dependent: pitch wander for tonal waves, lowpass cutoff
    // for white/pink, bit-depth for LFSR. Raw 0..1 value stored here.
    void setDrift (float d)     noexcept { driftAmt = d; }
    // Symmetry: bipolar -1..+1. Meaning depends on waveform (see tick()).
    void setSymmetry (float s)  noexcept { symmetry = s; }

    float tick() noexcept
    {
        // ----- pitch drift (tonal waveforms only) -----
        if (driftCount <= 0)
        {
            driftCount = (int) (sampleRate * (0.03 + 0.05 * rngFloat()));
            driftTarget = (rngFloat() * 2.0f - 1.0f);
        }
        --driftCount;
        driftState += (driftTarget - driftState) * 0.0008f;

        const bool isNoise = (shape == White || shape == Pink || shape == LFSR);

        // pitch drift only applies to tonal waves; noise uses Drift differently
        float driftedHz = freq;
        if (! isNoise)
            driftedHz = freq * (1.0f + driftAmt * 0.02f * driftState);

        double inc = (double) driftedHz / sampleRate;
        phase += inc;
        while (phase >= 1.0) phase -= 1.0;
        while (phase < 0.0)  phase += 1.0;

        float raw = 0.0f;

        switch (shape)
        {
            case Sine:
            {
                // Symmetry skews the phase (phase distortion): compresses one
                // half-cycle and stretches the other -> asymmetric, even harmonics.
                float p = (float) phase;
                float bend = symmetry * 0.45f;          // -0.45..0.45
                float pivot = 0.5f + bend;
                float wp;
                if (p < pivot) wp = 0.5f * (p / pivot);
                else           wp = 0.5f + 0.5f * ((p - pivot) / (1.0f - pivot));
                raw = std::sin (wp * BZ_TWO_PI);
                break;
            }

            case Triangle:
            {
                // Symmetry moves the peak position (0..1). Clamp away from the
                // exact edges so it stays a triangle, not a pure ramp.
                float peak = 0.5f + symmetry * 0.48f;   // 0.02..0.98
                float p = (float) phase;
                if (p < peak) raw = -1.0f + 2.0f * (p / peak);
                else          raw =  1.0f - 2.0f * ((p - peak) / (1.0f - peak));
                break;
            }

            case Sawtooth:
            {
                // Rising saw with a phase-warp from Symmetry: the warp bends the
                // ramp convex/concave while keeping the hard reset that defines a
                // sawtooth. Never cancels, always has the characteristic edge.
                float p = (float) phase;
                float warp = symmetry * 0.6f;
                // warp phase: positive bends the ramp to linger low then rush up
                float wp = p + warp * p * (1.0f - p) * 2.0f;
                if (wp < 0.0f) wp = 0.0f; if (wp > 1.0f) wp = 1.0f;
                raw = 2.0f * wp - 1.0f;
                break;
            }

            case Pulse:
            {
                // Symmetry sets pulse width. Mapped to a range that always stays
                // audible: above ~72% duty the pulse spends too little time low
                // and reads as near-silent through the ring mod, so the full knob
                // travel is compressed into roughly 5%..72%.
                float width = 0.385f + symmetry * 0.335f;   // -1 -> 0.05, +1 -> 0.72
                raw = ((float) phase < width) ? 1.0f : -1.0f;
                // soften edges slightly so it's not a perfectly digital square
                raw = std::tanh (raw * 3.0f);
                break;
            }

            case White:
            {
                float w = whiteSample();
                raw = applyNoiseDriftLP (w);
                raw = applyNoiseSymmetry (raw);
                break;
            }

            case Pink:
            {
                float w = whiteSample();
                // Paul Kellet refined pink filter
                pinkB[0] = 0.99886f * pinkB[0] + w * 0.0555179f;
                pinkB[1] = 0.99332f * pinkB[1] + w * 0.0750759f;
                pinkB[2] = 0.96900f * pinkB[2] + w * 0.1538520f;
                pinkB[3] = 0.86650f * pinkB[3] + w * 0.3104856f;
                pinkB[4] = 0.55000f * pinkB[4] + w * 0.5329522f;
                pinkB[5] = -0.7616f * pinkB[5] - w * 0.0168980f;
                float pink = pinkB[0] + pinkB[1] + pinkB[2] + pinkB[3]
                           + pinkB[4] + pinkB[5] + pinkB[6] + w * 0.5362f;
                pinkB[6] = w * 0.115926f;
                raw = applyNoiseDriftLP (pink * 0.11f);
                raw = applyNoiseSymmetry (raw);
                break;
            }

            case LFSR:
            {
                // Bit depth set by Drift (3..16): low = short, pitched, buzzy;
                // high = long-period rumble. The shift register is clocked at a
                // rate derived from the carrier frequency.
                int depth = 3 + (int) (driftAmt * 13.0f + 0.5f);
                if (depth < 3)  depth = 3;
                if (depth > 16) depth = 16;
                uint32_t mask = (1u << depth) - 1u;

                // clock the LFSR at ~ frequency * depth steps/sec, advanced by phase
                lfsrPhase += inc * (float) depth;
                while (lfsrPhase >= 1.0)
                {
                    lfsrPhase -= 1.0;
                    uint32_t lsb = lfsrReg & 1u;
                    lfsrReg >>= 1;
                    if (lsb) lfsrReg ^= (lfsrTap (depth) & mask);
                    lfsrReg &= mask;
                    if (lfsrReg == 0u) lfsrReg = 1u;
                    lfsrHeld = ((float) lfsrReg / (float) (mask + 1u)) * 2.0f - 1.0f;
                }

                raw = lfsrHeld;

                // Symmetry = sample-rate reduction / bitcrush hold for extra
                // digital artifacts.
                if (symmetry > 0.0f)
                {
                    float holdHz = 200.0f + (1.0f - symmetry) * 20000.0f;
                    srPhase += holdHz / sampleRate;
                    if (srPhase >= 1.0) { srPhase -= 1.0; srHold = raw; }
                    raw = srHold;
                }
                else if (symmetry < 0.0f)
                {
                    // negative side: quantise amplitude (bit reduction)
                    float levels = 2.0f + (1.0f + symmetry) * 30.0f; // 2..32
                    raw = std::round (raw * levels) / levels;
                }
                break;
            }
        }

        // ----- triode-style asymmetric soft saturation (Drive) -----
        float k = 1.0f + drive * 4.0f;
        float driven = raw * k;
        float sat;
        if (driven >= 0.0f) sat = std::tanh (driven);
        else                sat = std::tanh (driven * 0.8f) * 0.92f;

        // DC blocker — the asymmetric saturation and some symmetry settings add
        // a DC component; remove it so the carrier stays centred.
        float dcOut = sat - dcX1 + 0.9995f * dcY1;
        dcX1 = sat;
        dcY1 = dcOut;
        return dcOut;
    }

private:
    float rngFloat() noexcept
    {
        rngz ^= rngz << 13; rngz ^= rngz >> 17; rngz ^= rngz << 5;
        return (float) ((rngz >> 8) & 0xFFFFFF) / (float) 0x1000000;
    }

    float whiteSample() noexcept
    {
        rngz ^= rngz << 13; rngz ^= rngz >> 17; rngz ^= rngz << 5;
        return ((float) ((rngz >> 8) & 0xFFFFFF) / (float) 0x800000) - 1.0f;
    }

    // Drift = lowpass cutoff for white/pink. At 1.0 fully open (bright),
    // toward 0 progressively darker — like a real analog noise source with a
    // tone control.
    float applyNoiseDriftLP (float x) noexcept
    {
        float cutoff = 0.0008f + driftAmt * driftAmt * 0.6f;  // smooth, dark..open
        noiseLP += cutoff * (x - noiseLP);
        return noiseLP * (1.0f + driftAmt * 1.5f);            // makeup as it opens
    }

    // Symmetry for noise = distribution skew: bias the noise lopsided so it
    // crackles asymmetrically (more positive or negative spikes).
    float applyNoiseSymmetry (float x) noexcept
    {
        if (symmetry == 0.0f) return x;
        // asymmetric power curve around 0
        float s = symmetry * 0.7f;
        if (x >= 0.0f) return std::pow (x, 1.0f - s);
        else           return -std::pow (-x, 1.0f + s);
    }

    static uint32_t lfsrTap (int depth) noexcept
    {
        // maximal-length Galois taps per bit width (period 2^n - 1)
        switch (depth)
        {
            case 3:  return 0b110u;
            case 4:  return 0b1100u;
            case 5:  return 0b10100u;
            case 6:  return 0b110000u;
            case 7:  return 0b1100000u;
            case 8:  return 0b10111000u;
            case 9:  return 0b100010000u;
            case 10: return 0b1001000000u;
            case 11: return 0b10100000000u;
            case 12: return 0b100000101001u;
            case 13: return 0b1000000001101u;
            case 14: return 0b10000000010101u;
            case 15: return 0b110000000000000u;
            default: return 0b1101000000001000u; // 16
        }
    }

    double sampleRate = 44100.0;
    double phase = 0.0;
    float  freq  = 440.0f;
    Shape  shape = Sine;
    float  drive = 0.3f;
    float  driftAmt = 0.3f;
    float  symmetry = 0.0f;

    float driftState = 0.0f, driftTarget = 0.0f;
    int   driftCount = 0;
    uint32_t rngz = 0x1234567u;

    // noise state
    float pinkB[7] = {0,0,0,0,0,0,0};
    float noiseLP = 0.0f;
    uint32_t lfsrReg = 0xACE1u;
    double lfsrPhase = 0.0;
    float lfsrHeld = 0.0f;
    float srHold = 0.0f;
    double srPhase = 0.0;
    float dcX1 = 0.0f, dcY1 = 0.0f;   // DC blocker state
};

// ============================================================
//  Diode shaping functions (static, memoryless)
// ============================================================

// Germanium: soft, low threshold, gentle expanding knee -> warm, rounded,
// lots of high-order content.
inline float shaperGe (float x, float vb) noexcept
{
    float d = x - vb;
    if (d <= 0.0f) return 0.0f;
    return d * d * (1.0f + 0.5f * d);
}

// Silicon: harder threshold, steeper then linear -> brighter, edgier.
inline float shaperSi (float x, float vb) noexcept
{
    float d = x - vb;
    if (d <= 0.0f) return 0.0f;
    if (d < 0.5f) return 4.0f * d * d * d;
    return 0.5f + 2.0f * (d - 0.5f);
}

// ============================================================
//  DiodeRing — stateless waveshaper ring modulator
// ============================================================
class DiodeRing
{
public:
    enum DiodeType { Germanium = 0, Silicon = 1 };

    void prepare (double sr) noexcept
    {
        sampleRate = sr;
        z1 = 0.0f;
        instPhase = 0.0f;
        prevMv = prevCv = 0.0f;
        inEnv = 0.0f;
        // fast attack (~2 ms), moderate release (~40 ms)
        atkCoeff = std::exp (-1.0f / (float) (sr * 0.002));
        relCoeff = std::exp (-1.0f / (float) (sr * 0.040));
    }

    void reset() noexcept { z1 = 0.0f; }

    void setOversample (int factor) noexcept { OS = factor < 1 ? 1 : (factor > 8 ? 8 : factor); }
    void setDiode (DiodeType d)     noexcept { diodeType = d; }
    void setImbalance (float v)     noexcept { imbalance = v; }
    void setInstability (float v)   noexcept { instability = v; }
    void setGateEnabled (bool g)    noexcept { gateEnabled = g; }
    void setInputLevel (float v)    noexcept { inLevel = v; }
    void setCarrierLevel (float v)  noexcept { carLevel = v; }

    // m = modulator (incoming audio), c = carrier; both ~ -1..1
    float process (float m, float c) noexcept
    {
        // --- input envelope follower (fast attack, moderate release) ---
        // Used to gate the wet output so the carrier is silent when there is
        // no input present.
        float ax = std::fabs (m);
        if (ax > inEnv) inEnv = atkCoeff * inEnv + (1.0f - atkCoeff) * ax;
        else            inEnv = relCoeff * inEnv + (1.0f - relCoeff) * ax;

        float mv = m * 0.6f * inLevel;
        float cv = c * 0.6f * carLevel;

        // instability: slowly wander the diode thresholds (bounded; no state runaway)
        float vbPos = baseVb;
        float vbNeg = baseVb;
        if (instability > 0.0001f)
        {
            instPhase += instRate;
            if (instPhase >= BZ_TWO_PI) instPhase -= BZ_TWO_PI;
            float w1 = std::sin (instPhase);
            float w2 = std::sin (instPhase * 1.37f + 0.6f);
            vbPos += instability * 0.05f * w1;
            vbNeg += instability * 0.05f * w2;
        }

        // imbalance: mismatch the two diode-pair gains and one threshold.
        // Kept subtle so the carrier null stays mostly intact — imbalance is
        // for harmonic character, not for spraying carrier everywhere.
        float gPos = 1.0f + imbalance * 0.15f;
        float gNeg = 1.0f - imbalance * 0.15f;
        vbNeg += imbalance * 0.05f;

        // oversampled, stateless shaping (linear-interp the inputs across sub-steps)
        float acc = 0.0f;
        float pm = prevMv, pc = prevCv;
        for (int n = 1; n <= OS; ++n)
        {
            float t = (float) n / (float) OS;
            float im = pm + (mv - pm) * t;
            float ic = pc + (cv - pc) * t;

            float up, dn;
            if (diodeType == Germanium)
            {
                up = shaperGe (ic + im, vbPos);
                dn = shaperGe (ic - im, vbNeg);
            }
            else
            {
                up = shaperSi (ic + im, vbPos);
                dn = shaperSi (ic - im, vbNeg);
            }
            float s = gPos * up - gNeg * dn;

            // instability fizz: small intermodulation spit, no feedback/state
            if (instability > 0.0001f)
                s += instability * 0.15f * up * dn;

            acc += s;
        }
        prevMv = mv;
        prevCv = cv;
        float out = acc / (float) OS;

        // gentle one-pole downsample LPF (cheap, unconditionally stable)
        z1 = z1 + dsCoeff * (out - z1);
        out = z1;

        out *= outGain;

        // --- input gate (optional) ---
        // When enabled, smoothly fade the wet signal to silence as input drops
        // below the gate floor, so the bare carrier is never heard on its own.
        if (gateEnabled)
        {
            float g = (inEnv - gateFloor * 0.3f) / gateFloor;
            g = g < 0.0f ? 0.0f : (g > 1.0f ? 1.0f : g);
            out *= g;
        }
        else
        {
            // Gate off: Benzene is a standalone sound source. Route the carrier
            // straight to the output so it's always audible regardless of how
            // well the ring nulls it (i.e. independent of Imbalance). The
            // ring-modulated content is still summed in, so input audio is still
            // processed if present.
            out += cv * 1.1f;
        }

        if (out >  1.0f) out =  1.0f + std::tanh (out - 1.0f) * 0.3f;
        if (out < -1.0f) out = -1.0f + std::tanh (out + 1.0f) * 0.3f;
        return out;
    }

private:
    double sampleRate = 44100.0;
    int    OS = 4;
    DiodeType diodeType = Germanium;
    float  imbalance = 0.3f;
    float  instability = 0.0f;
    bool   gateEnabled = true;
    float  inLevel = 1.0f;
    float  carLevel = 1.0f;

    static constexpr float baseVb   = 0.20f;
    static constexpr float outGain  = 1.6f;
    static constexpr float dsCoeff  = 0.6f;
    static constexpr float instRate = 0.00035f;
    static constexpr float gateFloor = 0.02f;   // input level below which wet mutes

    float prevMv = 0.0f, prevCv = 0.0f;
    float z1 = 0.0f;
    float instPhase = 0.0f;
    float inEnv = 0.0f;                 // input envelope follower
    float atkCoeff = 0.0f, relCoeff = 0.0f;
};

} // namespace bz
