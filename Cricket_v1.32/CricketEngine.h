#pragma once
#include <cstdint>
#include <cmath>
#include <cstring>

// ============================================================
//  Wavetables — ported directly from waves.h
// ============================================================
static constexpr int PHASE_SZ = 256;

static const int8_t wt_saw[PHASE_SZ] = {
    -128,-127,-126,-125,-124,-123,-122,-121,-120,-119,-118,
    -117,-116,-115,-114,-113,-112,-111,-110,-109,-108,-107,
    -106,-105,-104,-103,-102,-101,-100,-99,-98,-97,-96,-95,
    -94,-93,-92,-91,-90,-89,-88,-87,-86,-85,-84,-83,-82,-81,
    -80,-79,-78,-77,-76,-75,-74,-73,-72,-71,-70,-69,-68,-67,
    -66,-65,-64,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,
    -52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,
    -38,-37,-36,-35,-34,-33,-32,-31,-30,-29,-28,-27,-26,-25,
    -24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,
    -10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,
    12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
    31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,
    50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,
    69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,
    88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,
    105,106,107,108,109,110,111,112,113,114,115,116,117,118,
    119,120,121,122,123,124,125,126,127
};

static const int8_t wt_squ50[PHASE_SZ] = {
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
    -128,-128,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127
};

static const int8_t wt_noise[PHASE_SZ] = {
    124,-118,98,105,75,-103,-61,-42,45,-93,56,-101,39,-2,71,54,102,99,-43,50,
    -78,-120,62,0,-6,103,28,30,91,77,19,-81,-67,98,-121,-3,-85,122,54,0,-8,
    -113,46,-117,-110,5,-103,81,80,56,-90,40,4,120,37,76,-12,-18,82,-107,-94,
    -84,-28,84,77,-113,-26,6,-22,39,32,-54,-18,-124,123,-85,-101,-33,-77,-3,
    -41,115,107,-115,60,-59,-20,12,112,-21,123,-51,51,42,9,50,42,-83,-95,127,
    -84,-120,15,97,43,-79,-34,-11,122,-88,90,36,-32,-79,-19,-5,-97,22,-70,-30,
    21,-64,-54,29,-60,82,123,58,-40,21,-101,103,96,81,-62,24,-122,-20,-48,-87,
    -82,-20,-104,25,-8,49,50,35,-119,-110,-47,7,39,-24,81,55,119,7,-45,-101,
    28,71,-20,-105,-60,-89,-56,-16,6,-11,95,4,113,35,116,-67,44,-54,43,49,
    -111,-63,-71,42,87,-40,71,44,-126,26,-29,106,-128,-10,-20,-10,68,-46,72,
    -8,-119,-83,56,-7,-89,-41,27,-79,60,-66,106,-59,67,-80,-55,-105,19,46,11,
    -19,36,37,45,34,113,-75,53,-68,-98,27,-13,-11,41,68,-39,41,-22,87,84,-63,
    28,20,10,94,-60,-47,-98,112,37,-6,35,11,37,11,56,5
};

static const int8_t wt_tri[PHASE_SZ] = {
    -128,-126,-124,-122,-120,-118,-116,-114,-112,-110,-108,-106,-104,-102,-100,
    -98,-96,-94,-92,-90,-88,-86,-84,-82,-80,-78,-76,-74,-72,-70,-68,-66,-64,
    -62,-60,-58,-56,-54,-52,-50,-48,-46,-44,-42,-40,-38,-36,-34,-32,-30,-28,
    -26,-24,-22,-20,-18,-16,-14,-12,-10,-8,-6,-4,-2,0,2,4,6,8,10,12,14,16,18,
    20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,
    68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,
    112,114,116,118,120,122,124,126,127,125,123,121,119,117,115,113,111,109,
    107,105,103,101,99,97,95,93,91,89,87,85,83,81,79,77,75,73,71,69,67,65,63,
    61,59,57,55,53,51,49,47,45,43,41,39,37,35,33,31,29,27,25,23,21,19,17,15,
    13,11,9,7,5,3,1,-1,-3,-5,-7,-9,-11,-13,-15,-17,-19,-21,-23,-25,-27,-29,-31,
    -33,-35,-37,-39,-41,-43,-45,-47,-49,-51,-53,-55,-57,-59,-61,-63,-65,-67,-69,
    -71,-73,-75,-77,-79,-81,-83,-85,-87,-89,-91,-93,-95,-97,-99,-101,-103,-105,
    -107,-109,-111,-113,-115,-117,-119,-121,-123,-125,-127
};

// Attack envelope (logarithmic rise 0->255)
static const uint8_t env_attack[PHASE_SZ] = {
    0,1,3,5,7,9,11,13,14,16,18,20,22,23,25,27,29,30,32,34,35,37,39,41,42,44,
    45,47,49,50,52,54,55,57,58,60,61,63,65,66,68,69,71,72,74,75,76,78,79,81,
    82,84,85,87,88,89,91,92,93,95,96,98,99,100,102,103,104,105,107,108,109,111,
    112,113,114,116,117,118,119,121,122,123,124,125,126,128,129,130,131,132,133,
    135,136,137,138,139,140,141,142,143,144,146,147,148,149,150,151,152,153,154,
    155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,170,171,172,
    173,174,175,176,177,178,179,179,180,181,182,183,184,185,185,186,187,188,189,
    190,190,191,192,193,194,194,195,196,197,198,198,199,200,201,201,202,203,204,
    204,205,206,206,207,208,209,209,210,211,211,212,213,213,214,215,216,216,217,
    218,218,219,219,220,221,221,222,223,223,224,225,225,226,226,227,228,228,229,
    229,230,231,231,232,232,233,233,234,235,235,236,236,237,237,238,238,239,239,
    240,240,241,242,242,243,243,244,244,245,245,246,246,247,247,248,248,249,249,
    249,250,250,251,251,252,252,253,253,254,254,255,255
};

// Exponential release (e^-t, 255->0)
static const uint8_t env_exp[PHASE_SZ] = {
    255,250,245,241,236,232,227,223,219,215,210,206,202,199,195,191,188,184,180,
    177,174,170,167,164,161,158,155,152,149,146,143,141,138,135,133,130,128,125,
    123,121,118,116,114,112,109,107,105,103,101,99,98,96,94,92,90,89,87,85,84,
    82,81,79,77,76,75,73,72,70,69,68,66,65,64,63,62,60,59,58,57,56,55,54,53,52,
    51,50,49,48,47,46,45,44,44,43,42,41,40,40,39,38,37,37,36,35,35,34,33,33,32,
    31,31,30,30,29,29,28,27,27,26,26,25,25,24,24,24,23,23,22,22,21,21,21,20,20,
    19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,15,14,14,14,13,13,13,13,12,12,
    12,12,12,11,11,11,11,11,10,10,10,10,10,9,9,9,9,9,9,8,8,8,8,8,8,7,7,7,7,7,
    7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,4,4,4,4,4,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0
};

enum WaveShape { WAVE_SAW=0, WAVE_TRI=1, WAVE_NOISE=2, WAVE_SQU50=3 };
enum LogicMode  { LOGIC_AND=0, LOGIC_OR=1, LOGIC_XOR=2 };
enum EnvState   { ENV_ATTACK=0, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE, ENV_END };

inline const int8_t* getWaveTable(WaveShape w) {
    switch(w) {
        case WAVE_SAW:   return wt_saw;
        case WAVE_TRI:   return wt_tri;
        case WAVE_NOISE: return wt_noise;
        default:         return wt_squ50;
    }
}

// ============================================================
//  CricketVoice
// ============================================================
class CricketVoice
{
public:
    bool    active   = false;
    bool    noteHeld = false;
    int     midiNote = -1;
    float   velocity = 1.0f;

    // Oscillator state (float phases for frequency accuracy)
    float osc1_phase = 0.0f;
    float osc2_phase = 0.0f;
    float LFO_phase  = 0.0f;

    // ============================================================
    //  Envelope state — direct sample-accurate ADSR, no table stepping
    // ============================================================
    EnvState envState  = ENV_END;
    float    envVol    = 0.0f;     // current amplitude 0..1
    float    envTarget = 0.0f;     // target for current stage
    float    envStart  = 0.0f;     // start level for current stage
    float    envCoeff  = 0.0f;     // per-sample multiplier for exp stages
    unsigned long env_ct    = 0;   // sample counter within stage
    unsigned long env_len   = 0;   // total samples for current stage

    float releaseStartVol = 0.0f;

    // Mod envelope — independent ADSR with loop, outputs modEnvVol 0..1
    EnvState     modState   = ENV_END;
    float        modVol     = 0.0f;
    float        modStart   = 0.0f;
    float        modCoeff   = 0.0f;
    unsigned long mod_ct    = 0;
    unsigned long mod_len   = 0;
    float         modReleaseStart = 0.0f;

    // Low-range divider counter (unused — loRange now baked into osc2_inc)
    unsigned int rngct = 0;

    struct Params {
        // Pitch: semitone offset from MIDI note (-24 to +24). 0 = MIDI-accurate.
        float   osc1_semitones = 0.0f;
        float   osc2_semitones = 0.0f;
        float   osc1_vol       = 1.0f;   // 0-1
        float   osc2_vol       = 0.0f;
        float   FM_depth       = 0.0f;   // 0-1
        float   LFO_hz         = 1.0f;   // LFO frequency in Hz
        float   LFO_depth      = 0.0f;   // 0-1
        float   scale          = 1.0f;   // wrap distortion multiplier (1=clean)
        // Envelope: all times in samples
        unsigned int attack    = 4410;   // 100ms at 44100
        unsigned int decay     = 4410;
        float        sustain   = 1.0f;   // 0-1 sustain level
        unsigned int release   = 22050;  // 500ms
        WaveShape   osc1Wave   = WAVE_SQU50;
        WaveShape   osc2Wave   = WAVE_SQU50;
        WaveShape   lfoWave    = WAVE_TRI;
        LogicMode   logicMode  = LOGIC_AND;
        bool        logicOn    = false;
        bool        loRange    = false;
        double      sampleRate = 44100.0;
        // Velocity sensitivity: 0=flat (velocity ignored), 1=fully sensitive
        float       velocitySensitivity = 0.5f;
        // Pitch bend: range in semitones per osc, raw bend value -1..+1
        float       osc1BendRange      = 2.0f;   // semitones
        float       osc2BendRange      = 2.0f;   // semitones
        float       pitchBendRaw       = 0.0f;   // -1..+1, from MIDI bend wheel
        // Mod envelope timing (in samples, same unit as amp env)
        unsigned int modAttack  = 4410;
        unsigned int modDecay   = 4410;
        float        modSustain = 0.0f;   // default 0 so it's a one-shot shape
        unsigned int modRelease = 4410;
        bool         modLoop    = false;  // if true, restarts attack when release ends
        // Mod amounts: how much the mod env (0..1) shifts each target
        float modToOsc1Pitch = 0.0f;  // semitones, bipolar -24..+24
        float modToOsc2Pitch = 0.0f;  // semitones, bipolar -24..+24
        float modToFMDepth   = 0.0f;  // additive 0..1
        float modToLFOSpeed  = 0.0f;  // additive Hz 0..60
        float modToWrap      = 0.0f;  // additive 0..7
        float modToCutoff    = 0.0f;  // additive normalised 0..1 (maps to full cutoff range)

        // Filter
        int   filterType     = 0;     // 0=bypass 1=MS20LP 2=MS20HP 3=comb
        float filterCutoff   = 1.0f;  // normalised 0..1 (maps to 20Hz..20kHz)
        float filterRes      = 0.0f;  // 0..1
        float lfoToCutoff    = 0.0f;  // LFO mod amount to cutoff, normalised ±1
        float lfoToRes       = 0.0f;  // LFO mod amount to resonance ±1
    } params;

    // Mod envelope output — 0..1, applied each tick to modulate params
    float modEnvVol = 0.0f;

    // Per-voice filter state
    float flt_s1 = 0.0f, flt_s2 = 0.0f;
    float flt_cut_smooth = 1.0f;   // one-pole smoothed cutoff (normalised 0..1)
    float flt_res_smooth = 0.0f;   // one-pole smoothed resonance
    static constexpr int COMB_MAX = 8192;
    float combBuf[COMB_MAX] = {};
    int   combWrite = 0;

    float tick()
    {
        if (envState == ENV_END) { active = false; return 0.0f; }

        const int8_t* osc1wt = getWaveTable(params.osc1Wave);
        const int8_t* osc2wt = getWaveTable(params.osc2Wave);
        const int8_t* lfowt  = getWaveTable(params.lfoWave);

        // Run amp envelope
        updateEnvelope();
        if (envState == ENV_END) return 0.0f;

        // Run mod envelope
        updateModEnvelope();

        // Apply mod envelope as a multiplier on top of the precomputed phase increments.
        // This keeps pitch knobs and computePhaseIncrements() as the sole authority on
        // base pitch — mod only applies an additional ratio on top.
        // No std::pow in the common case (modVol == 0 or amounts == 0).
        float eff_fm       = params.FM_depth;
        float eff_scale    = params.scale;
        float eff_osc1_inc = osc1_inc;
        float eff_osc2_inc = osc2_inc;
        float eff_lfo_inc  = lfo_inc;

        if (modVol > 0.0001f) {
            // Pitch modulation: multiply increment by 2^(semitones/12)
            if (params.modToOsc1Pitch != 0.0f)
                eff_osc1_inc = osc1_inc * std::pow(2.0f, (modVol * params.modToOsc1Pitch) / 12.0f);
            if (params.modToOsc2Pitch != 0.0f && !params.loRange)
                eff_osc2_inc = osc2_inc * std::pow(2.0f, (modVol * params.modToOsc2Pitch) / 12.0f);

            // Other modulations
            eff_fm    = std::fmin(1.0f, params.FM_depth + modVol * params.modToFMDepth);
            eff_scale = params.scale + modVol * params.modToWrap;
            // (modToCutoff applied directly in filter section below)

            if (params.modToLFOSpeed != 0.0f) {
                float modHz = std::fmax(0.0f, params.LFO_hz + modVol * params.modToLFOSpeed);
                eff_lfo_inc = modHz * ((float)PHASE_SZ / (float)params.sampleRate);
            }
        }

        // Read wavetable samples as signed bipolar (-1..+1)
        float o1_sig = osc1wt[(int)osc1_phase & (PHASE_SZ-1)] / 128.0f;
        float o2_sig = osc2wt[(int)osc2_phase & (PHASE_SZ-1)] / 128.0f;

        // Velocity sensitivity
        float effectiveVel = 1.0f - params.velocitySensitivity
                           + params.velocitySensitivity * velocity;

        float o1_vol = params.osc1_vol * envVol * effectiveVel;
        float o2_vol = params.osc2_vol * envVol * effectiveVel;

        float mixed;
        if (!params.logicOn) {
            mixed = o1_sig * o1_vol + o2_sig * o2_vol;
        } else {
            uint8_t u1 = (uint8_t)(osc1wt[(int)osc1_phase & (PHASE_SZ-1)] + 0x80);
            uint8_t u2 = (uint8_t)(osc2wt[(int)osc2_phase & (PHASE_SZ-1)] + 0x80);
            unsigned int result;
            switch (params.logicMode) {
                case LOGIC_AND: result = u1 & u2; break;
                case LOGIC_OR:  result = u1 | u2; break;
                default:        result = u1 ^ u2; break;
            }
            mixed = ((result / 255.0f) * 2.0f - 1.0f) * envVol * velocity;
        }

        // Apply wrap with mod-env contribution
        float out = mixed * std::fmax(1.0f, eff_scale);
        out = std::fmax(-1.0f, std::fmin(1.0f, out));

        // Advance phases using modulated increments
        advancePhases(osc1wt, osc2wt, lfowt, eff_osc1_inc, eff_osc2_inc, eff_lfo_inc, eff_fm);

        // ---- Filter ----
        if (params.filterType != 0) {
            // Compute LFO value for filter modulation (use current LFO phase)
            float lfoVal = lfowt[(int)LFO_phase & (PHASE_SZ-1)] / 128.0f; // -1..+1

            // Effective cutoff and resonance (clamped 0..1)
            float eff_cut = std::fmax(0.0f, std::fmin(1.0f,
                params.filterCutoff
                + lfoVal * params.lfoToCutoff
                + (modVol > 0.0001f ? modVol * params.modToCutoff : 0.0f)));
            float eff_res = std::fmax(0.0f, std::fmin(1.0f,
                params.filterRes + lfoVal * params.lfoToRes));

            out = applyFilter(out, eff_cut, eff_res);
        }

        return out;
    }

    void noteOn(int note, float vel) {
        midiNote  = note;
        velocity  = vel;
        active    = true;
        noteHeld  = true;
        // Reset filter state to prevent bleed between notes
        flt_s1 = flt_s2 = 0.0f;
        flt_cut_smooth = params.filterCutoff;
        flt_res_smooth = params.filterRes;
        std::fill(combBuf, combBuf + COMB_MAX, 0.0f);
        combWrite = 0;
        computePhaseIncrements();
        startAttack();
        startModAttack();
    }

    void noteOff() {
        noteHeld = false;
        if (envState != ENV_END && envState != ENV_RELEASE)
            startRelease();
        if (modState != ENV_END && modState != ENV_RELEASE)
            startModRelease();
    }

    bool isFinished() const { return envState == ENV_END; }

    // Phase increments — set by computePhaseIncrements(), read by tick()/advancePhases()
    float osc1_inc = 1.0f;
    float osc2_inc = 1.0f;
    float lfo_inc  = 0.0f;

    // Recompute pitch phase increments — call once per block after params update
    void computePhaseIncrements() {
        if (midiNote < 0) return;
        const float sr     = (float)params.sampleRate;
        const float ratio  = (float)PHASE_SZ / sr;

        // OSC 1: MIDI note + semitone offset + pitch bend (range × raw value)
        float bend1 = params.pitchBendRaw * params.osc1BendRange;
        float freq1 = 440.0f * std::pow(2.0f,
            (midiNote - 69 + params.osc1_semitones + bend1) / 12.0f);
        freq1 = std::fmax(20.0f, std::fmin(20000.0f, freq1));
        osc1_inc = freq1 * ratio;

        // OSC 2: High range = audio rate; Low range = sub-audio (bend not applied in low range)
        if (params.loRange) {
            float lfoBase = 1.0f;
            float osc2Hz  = lfoBase * std::pow(2.0f, params.osc2_semitones / 12.0f);
            osc2Hz = std::fmax(0.01f, std::fmin(30.0f, osc2Hz));
            osc2_inc = osc2Hz * ratio;
        } else {
            float bend2 = params.pitchBendRaw * params.osc2BendRange;
            float freq2 = 440.0f * std::pow(2.0f,
                (midiNote - 69 + params.osc2_semitones + bend2) / 12.0f);
            freq2 = std::fmax(20.0f, std::fmin(20000.0f, freq2));
            osc2_inc = freq2 * ratio;
        }

        // LFO always runs at its own Hz
        lfo_inc = params.LFO_hz * ratio;
    }

private:
    // ---- Amp envelope helpers (unchanged) ----
    void startAttack() {
        envState = ENV_ATTACK;
        envStart = envVol;
        env_ct   = 0;
        env_len  = std::max(1u, params.attack);
    }

    void startDecay() {
        envState = ENV_DECAY;
        envStart = envVol;
        env_ct   = 0;
        env_len  = std::max(1u, params.decay);
        float target = std::fmax(0.0001f, params.sustain);
        envCoeff = std::exp(std::log(target / std::fmax(0.0001f, envStart))
                            / (float)env_len);
    }

    void startRelease() {
        releaseStartVol = envVol;
        envState = ENV_RELEASE;
        env_ct   = 0;
        env_len  = std::max(1u, params.release);
        if (releaseStartVol > 0.001f) {
            envCoeff = std::exp(std::log(0.0001f / releaseStartVol) / (float)env_len);
        } else {
            unsigned long shortLen = (env_len < 441u) ? env_len : 441u;
            env_len  = shortLen;
            envCoeff = std::exp(std::log(0.0001f / 0.001f) / (float)env_len);
        }
    }

    void updateEnvelope() {
        switch (envState) {
            case ENV_ATTACK:
                envVol = envStart + (1.0f - envStart) * ((float)env_ct / (float)env_len);
                if (++env_ct >= env_len) {
                    envVol = 1.0f;
                    if (noteHeld) startDecay();
                    else          startRelease();
                }
                break;
            case ENV_DECAY:
                envVol *= envCoeff;
                if (++env_ct >= env_len) {
                    envVol   = params.sustain;
                    envState = ENV_SUSTAIN;
                }
                break;
            case ENV_SUSTAIN:
                envVol = params.sustain;
                if (!noteHeld) startRelease();
                break;
            case ENV_RELEASE:
                envVol *= envCoeff;
                ++env_ct;
                if (env_ct >= env_len || envVol < 0.0001f) {
                    envState = ENV_END;
                    active   = false;
                }
                break;
            case ENV_END:
                envVol = 0.0f;
                active = false;
                break;
        }
    }

    // ---- Mod envelope helpers ----
    void startModAttack() {
        modState = ENV_ATTACK;
        modStart = modVol;
        mod_ct   = 0;
        mod_len  = std::max(1u, params.modAttack);
    }

    void startModDecay() {
        modState = ENV_DECAY;
        modStart = modVol;
        mod_ct   = 0;
        mod_len  = std::max(1u, params.modDecay);
        float target = std::fmax(0.0001f, params.modSustain);
        modCoeff = std::exp(std::log(target / std::fmax(0.0001f, modStart))
                            / (float)mod_len);
    }

    void startModRelease() {
        modReleaseStart = modVol;
        modState = ENV_RELEASE;
        mod_ct   = 0;
        mod_len  = std::max(1u, params.modRelease);
        if (modReleaseStart > 0.001f)
            modCoeff = std::exp(std::log(0.0001f / modReleaseStart) / (float)mod_len);
        else {
            unsigned long shortLen = (mod_len < 441u) ? mod_len : 441u;
            mod_len  = shortLen;
            modCoeff = std::exp(std::log(0.0001f / 0.001f) / (float)mod_len);
        }
    }

    void updateModEnvelope() {
        switch (modState) {
            case ENV_ATTACK:
                modVol = modStart + (1.0f - modStart) * ((float)mod_ct / (float)mod_len);
                if (++mod_ct >= mod_len) {
                    modVol = 1.0f;
                    if (noteHeld) startModDecay();
                    else          startModRelease();
                }
                break;
            case ENV_DECAY:
                modVol *= modCoeff;
                if (++mod_ct >= mod_len) {
                    modVol = params.modSustain;
                    // If looping and sustain is zero, skip sustain and restart immediately
                    if (params.modLoop && noteHeld && params.modSustain <= 0.0001f) {
                        startModAttack();
                    } else {
                        modState = ENV_SUSTAIN;
                    }
                }
                break;
            case ENV_SUSTAIN:
                modVol = params.modSustain;
                if (!noteHeld) {
                    startModRelease();
                } else if (params.modLoop && params.modSustain <= 0.0001f) {
                    // Sustain at zero with loop — restart attack
                    startModAttack();
                }
                break;
            case ENV_RELEASE:
                modVol *= modCoeff;
                ++mod_ct;
                if (mod_ct >= mod_len || modVol < 0.0001f) {
                    modVol   = 0.0f;
                    modState = ENV_END;
                    if (params.modLoop && noteHeld)
                        startModAttack();
                }
                break;
            case ENV_END:
                modVol = 0.0f;
                if (params.modLoop && noteHeld)
                    startModAttack();
                break;
        }
    }

    // ---------------------------------------------------------------
    //  Inline saturation: asymmetric JFET-style soft clip
    //  Used on LP output for nasal MS-20 tone colour.
    // ---------------------------------------------------------------
    static inline float jfetClip(float x)
    {
        if (x >= 0.0f)
            return x / (1.0f + x * 0.5f);
        else
            return x / (1.0f - x * 0.3f);
    }

    // ---------------------------------------------------------------
    //  Filter processing
    //  eff_cut: normalised 0..1 (maps exponentially to 20Hz..20kHz)
    //  eff_res: 0..1 (0=none, self-oscillation at 1)
    //
    //  MS-20 LP/HP: Zavalishin TPT SVF with simultaneous implicit solve.
    //  Nonlinearity applied on STATE UPDATE (tanh on states), not on
    //  integrator outputs. This preserves the linear solve's identity
    //  (HP + 2R*BP + LP = input) at all resonance settings while still
    //  bounding self-oscillation through saturated state feedback.
    //
    //  Resonance curve: twoR = 2*(1-r)^2 gives squared damping —
    //    r=0.0 → Q=0.5 (Butterworth-like, no resonance)
    //    r=0.5 → Q=2.0 (strong resonance)
    //    r=0.9 → Q=50  (near self-oscillation)
    //    r=1.0 → Q=inf (self-oscillation, bounded by tanh on states)
    // ---------------------------------------------------------------
    float applyFilter(float in, float eff_cut, float eff_res)
    {
        const float sr = (float)params.sampleRate;

        // 30Hz one-pole smoother: fast enough for LFO mod, slow enough
        // to eliminate zipper noise and LFO discontinuity clicks.
        const float sc = std::exp(-2.0f * 3.14159265f * 30.0f / sr);
        flt_cut_smooth = sc * flt_cut_smooth + (1.0f - sc) * std::fmax(0.0f, std::fmin(1.0f, eff_cut));
        flt_res_smooth = sc * flt_res_smooth + (1.0f - sc) * std::fmax(0.0f, std::fmin(1.0f, eff_res));

        switch (params.filterType) {

        case 1: // MS-20 Lowpass
        case 2: // MS-20 Highpass
        {
            float cutHz = 20.0f * std::pow(1000.0f, flt_cut_smooth);
            cutHz = std::fmax(20.0f, std::fmin(cutHz, sr * 0.48f));

            float g = std::tan(3.14159265f * cutHz / sr);

            float r = flt_res_smooth;

            // Damping: squared (1-r) curve for early resonance onset.
            // twoR=2 at r=0 (Q=0.5), twoR=0 at r=1 (self-oscillation).
            float omr  = 1.0f - r;
            float twoR = 2.0f * omr * omr;

            // ---- Zavalishin TPT SVF: simultaneous implicit solve ----
            float denom = 1.0f / (1.0f + twoR * g + g * g);
            float hp = (in - (twoR + g) * flt_s1 - flt_s2) * denom;
            float bp = g * hp + flt_s1;
            float lp = g * bp + flt_s2;

            // ---- State update: tanh saturation ----
            // Bounds self-oscillation amplitude and adds harmonic content.
            // At low resonance, states are small, tanh is ~linear, filter is clean.
            flt_s1 = std::tanh(2.0f * bp - flt_s1);
            flt_s2 = std::tanh(2.0f * lp - flt_s2);

            if (params.filterType == 1)
            {
                // LP output: JFET asymmetric clip for nasal MS-20 character.
                // Drive scales with resonance so the resonant peak produces
                // more harmonics as you turn it up.
                float drive = 1.0f + r * 3.0f;
                return std::fmax(-1.0f, std::fmin(1.0f, jfetClip(lp * drive)));
            }
            else
            {
                // HP output: tanh saturation with resonance-scaled drive.
                float drive = 1.0f + r * 2.5f;
                return std::fmax(-1.0f, std::fmin(1.0f, std::tanh(hp * drive)));
            }
        }

        case 3: // Comb filter
        {
            float cutHz = 20.0f * std::pow(1000.0f, flt_cut_smooth);
            float delayLen = sr / std::fmax(1.0f, cutHz);
            int delaySamples = (int)std::fmin(delayLen, (float)(COMB_MAX - 1));

            int readPos = (combWrite - delaySamples + COMB_MAX) % COMB_MAX;
            float delayed = combBuf[readPos];
            float fb = flt_res_smooth * 0.99f;
            float out = in + fb * delayed;
            combBuf[combWrite] = out;
            combWrite = (combWrite + 1) % COMB_MAX;
            return std::fmax(-1.0f, std::fmin(1.0f, out));
        }

        default:
            return in;
        }
    }

    void advancePhases(const int8_t* /*osc1wt*/, const int8_t* osc2wt,
                       const int8_t* lfowt,
                       float eff_osc1_inc, float eff_osc2_inc,
                       float eff_lfo_inc, float eff_fm)
    {
        // LFO: bipolar -1..+1, scaled by depth
        float lfoVal = lfowt[(int)LFO_phase & (PHASE_SZ-1)] / 128.0f;
        float lfoMod = lfoVal * params.LFO_depth;

        // FM: osc2 → osc1 frequency modulation (uses modulated FM depth)
        float fmVal = osc2wt[(int)osc2_phase & (PHASE_SZ-1)] / 128.0f;
        float fmMod = fmVal * eff_fm * eff_osc1_inc * 4.0f;

        // Advance oscillators with modulated increments
        osc1_phase += eff_osc1_inc * (1.0f + lfoMod * 0.5f) + fmMod;
        while (osc1_phase >= PHASE_SZ) osc1_phase -= PHASE_SZ;
        while (osc1_phase <  0.0f)     osc1_phase += PHASE_SZ;

        osc2_phase += eff_osc2_inc * (1.0f + lfoMod * 0.5f);
        while (osc2_phase >= PHASE_SZ) osc2_phase -= PHASE_SZ;
        while (osc2_phase <  0.0f)     osc2_phase += PHASE_SZ;

        LFO_phase += eff_lfo_inc;
        while (LFO_phase >= PHASE_SZ) LFO_phase -= PHASE_SZ;
    }
};
