#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
CricketAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Waveforms
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::osc1Wave, "Osc 1 Wave",
        juce::StringArray{"Saw","Triangle","Noise","Square"}, 3));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::osc2Wave, "Osc 2 Wave",
        juce::StringArray{"Saw","Triangle","Noise","Square"}, 3));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::lfoWave, "LFO Wave",
        juce::StringArray{"Saw","Triangle","Noise","Square"}, 1));

    // Pitch: semitone offset -24..+24, default 0 (centre = MIDI-accurate)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::osc1Pitch, "Osc 1 Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::osc2Pitch, "Osc 2 Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));

    // Osc mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::oscMix, "Osc 1/2 Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    // ADSR — times in milliseconds, skewed toward short values
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::attack,  "Attack",
        juce::NormalisableRange<float>(1.0f, 4000.0f, 1.0f, 0.35f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::decay,   "Decay",
        juce::NormalisableRange<float>(1.0f, 4000.0f, 1.0f, 0.35f), 200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::sustain, "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::release_, "Release",
        juce::NormalisableRange<float>(1.0f, 8000.0f, 1.0f, 0.35f), 300.0f));

    // FM depth
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::fmDepth, "FM Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // LFO speed: 0.1 Hz to 60 Hz, skewed toward low end
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::lfoSpeed, "LFO Speed",
        juce::NormalisableRange<float>(0.1f, 60.0f, 0.01f, 0.4f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::lfoDepth, "LFO Depth",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    // Wrap: 1.0 (clean) to 8.0 (heavy distortion), exponential feel via skew
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::wrap, "Wrap",
        juce::NormalisableRange<float>(1.0f, 8.0f, 0.01f, 0.5f), 1.0f));

    // Logic
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamID::logicOn, "Logic On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::logicMode, "Logic Mode",
        juce::StringArray{"AND","OR","XOR"}, 0));

    // Osc 2 options
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamID::osc2Range, "Osc 2 Low Range", false));

    // Velocity sensitivity: 0=flat, 1=fully velocity-sensitive
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::velSensitivity, "Velocity Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    // Pitch bend range per oscillator: 0–12 semitones
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::osc1BendRange, "Osc 1 Bend Range",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::osc2BendRange, "Osc 2 Bend Range",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 2.0f));

    // Mod envelope ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modAttack,   "Mod Attack",
        juce::NormalisableRange<float>(1.0f, 4000.0f, 1.0f, 0.35f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modDecay,    "Mod Decay",
        juce::NormalisableRange<float>(1.0f, 4000.0f, 1.0f, 0.35f), 200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modSustain,  "Mod Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modRelease_, "Mod Release",
        juce::NormalisableRange<float>(1.0f, 8000.0f, 1.0f, 0.35f), 200.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamID::modLoop, "Mod Loop", false));
    // Mod amounts (bipolar for pitch, unipolar for the rest)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modToOsc1Pitch, "Mod->Osc1 Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modToOsc2Pitch, "Mod->Osc2 Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modToFMDepth, "Mod->FM Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modToLFOSpeed, "Mod->LFO Speed",
        juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f, 0.4f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modToWrap, "Mod->Wrap",
        juce::NormalisableRange<float>(0.0f, 7.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modToCutoff, "Mod->Cutoff",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // Filter
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::filterType, "Filter Type",
        juce::StringArray{"Bypass","MS20 LP","MS20 HP","Comb"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::filterCutoff, "Filter Cutoff",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));  // default open (20kHz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::filterRes, "Filter Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::lfoToCutoff, "LFO->Cutoff",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::lfoToRes, "LFO->Res",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    // Master volume
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::masterVol, "Master Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    return { params.begin(), params.end() };
}

CricketAudioProcessor::CricketAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "CricketState", createParameters())
{
    p_osc1Wave   = apvts.getRawParameterValue(ParamID::osc1Wave);
    p_osc2Wave   = apvts.getRawParameterValue(ParamID::osc2Wave);
    p_lfoWave    = apvts.getRawParameterValue(ParamID::lfoWave);
    p_osc1Pitch  = apvts.getRawParameterValue(ParamID::osc1Pitch);
    p_osc2Pitch  = apvts.getRawParameterValue(ParamID::osc2Pitch);
    p_oscMix     = apvts.getRawParameterValue(ParamID::oscMix);
    p_attack     = apvts.getRawParameterValue(ParamID::attack);
    p_decay      = apvts.getRawParameterValue(ParamID::decay);
    p_sustain    = apvts.getRawParameterValue(ParamID::sustain);
    p_release    = apvts.getRawParameterValue(ParamID::release_);
    p_fmDepth    = apvts.getRawParameterValue(ParamID::fmDepth);
    p_lfoSpeed   = apvts.getRawParameterValue(ParamID::lfoSpeed);
    p_lfoDepth   = apvts.getRawParameterValue(ParamID::lfoDepth);
    p_wrap       = apvts.getRawParameterValue(ParamID::wrap);
    p_logicOn    = apvts.getRawParameterValue(ParamID::logicOn);
    p_logicMode  = apvts.getRawParameterValue(ParamID::logicMode);
    p_osc2Range  = apvts.getRawParameterValue(ParamID::osc2Range);
    p_masterVol      = apvts.getRawParameterValue(ParamID::masterVol);
    p_velSensitivity = apvts.getRawParameterValue(ParamID::velSensitivity);
    p_osc1BendRange  = apvts.getRawParameterValue(ParamID::osc1BendRange);
    p_osc2BendRange  = apvts.getRawParameterValue(ParamID::osc2BendRange);
    p_modAttack      = apvts.getRawParameterValue(ParamID::modAttack);
    p_modDecay       = apvts.getRawParameterValue(ParamID::modDecay);
    p_modSustain     = apvts.getRawParameterValue(ParamID::modSustain);
    p_modRelease     = apvts.getRawParameterValue(ParamID::modRelease_);
    p_modLoop        = apvts.getRawParameterValue(ParamID::modLoop);
    p_modToOsc1Pitch = apvts.getRawParameterValue(ParamID::modToOsc1Pitch);
    p_modToOsc2Pitch = apvts.getRawParameterValue(ParamID::modToOsc2Pitch);
    p_modToFMDepth   = apvts.getRawParameterValue(ParamID::modToFMDepth);
    p_modToLFOSpeed  = apvts.getRawParameterValue(ParamID::modToLFOSpeed);
    p_modToWrap      = apvts.getRawParameterValue(ParamID::modToWrap);
    p_modToCutoff    = apvts.getRawParameterValue(ParamID::modToCutoff);
    p_filterType     = apvts.getRawParameterValue(ParamID::filterType);
    p_filterCutoff   = apvts.getRawParameterValue(ParamID::filterCutoff);
    p_filterRes      = apvts.getRawParameterValue(ParamID::filterRes);
    p_lfoToCutoff    = apvts.getRawParameterValue(ParamID::lfoToCutoff);
    p_lfoToRes       = apvts.getRawParameterValue(ParamID::lfoToRes);

    for (int i = 0; i < NUM_VOICES; ++i)
        voiceNoteMap[i] = -1;
}

CricketAudioProcessor::~CricketAudioProcessor() {}

void CricketAudioProcessor::fillVoiceParams(CricketVoice::Params& p)
{
    p.osc1Wave = (WaveShape)(int)*p_osc1Wave;
    p.osc2Wave = (WaveShape)(int)*p_osc2Wave;
    p.lfoWave  = (WaveShape)(int)*p_lfoWave;

    // Semitone offsets (-24..+24)
    p.osc1_semitones = *p_osc1Pitch;
    p.osc2_semitones = *p_osc2Pitch;

    // Mix
    float mix  = *p_oscMix;
    p.osc2_vol = mix;
    p.osc1_vol = 1.0f - mix;

    // ADSR: convert ms to samples
    double sr = p.sampleRate;
    p.attack  = (unsigned int)(*p_attack  * 0.001 * sr);
    p.decay   = (unsigned int)(*p_decay   * 0.001 * sr);
    p.sustain = *p_sustain;
    p.release = (unsigned int)(*p_release * 0.001 * sr);
    if (p.attack  < 1) p.attack  = 1;
    if (p.decay   < 1) p.decay   = 1;
    if (p.release < 1) p.release = 1;

    // Modulation
    p.FM_depth  = *p_fmDepth;
    p.LFO_hz    = *p_lfoSpeed;   // already in Hz from param range
    p.LFO_depth = *p_lfoDepth;

    // Wrap: param range 1-8, maps directly to scale multiplier
    p.scale = *p_wrap;

    // Logic
    p.logicOn   = (*p_logicOn > 0.5f);
    p.logicMode = (LogicMode)(int)*p_logicMode;

    // Velocity sensitivity and pitch bend
    p.velocitySensitivity = *p_velSensitivity;
    p.osc1BendRange       = *p_osc1BendRange;
    p.osc2BendRange       = *p_osc2BendRange;
    p.pitchBendRaw        = currentPitchBendRaw;

    // Mod envelope
    p.modAttack   = (unsigned int)(*p_modAttack  * 0.001 * p.sampleRate);
    p.modDecay    = (unsigned int)(*p_modDecay   * 0.001 * p.sampleRate);
    p.modSustain  = *p_modSustain;
    p.modRelease  = (unsigned int)(*p_modRelease * 0.001 * p.sampleRate);
    p.modLoop     = (*p_modLoop > 0.5f);
    if (p.modAttack  < 1) p.modAttack  = 1;
    if (p.modDecay   < 1) p.modDecay   = 1;
    if (p.modRelease < 1) p.modRelease = 1;
    // Mod amounts
    p.modToOsc1Pitch = *p_modToOsc1Pitch;
    p.modToOsc2Pitch = *p_modToOsc2Pitch;
    p.modToFMDepth   = *p_modToFMDepth;
    p.modToLFOSpeed  = *p_modToLFOSpeed;
    p.modToWrap      = *p_modToWrap;
    p.modToCutoff    = *p_modToCutoff;

    // Filter
    p.filterType   = (int)*p_filterType;
    p.filterCutoff = *p_filterCutoff;
    p.filterRes    = *p_filterRes;
    p.lfoToCutoff  = *p_lfoToCutoff;
    p.lfoToRes     = *p_lfoToRes;

    // Switches
    p.loRange    = (*p_osc2Range  > 0.5f);
}

int CricketAudioProcessor::allocateVoice()
{
    // 1. Free voice (fully finished)
    for (int i = 0; i < NUM_VOICES; ++i)
        if (voices[i].envState == ENV_END) return i;

    // 2. Quietest releasing voice (noteHeld=false, in release)
    int best = -1;
    float bestVol = 1.0f;
    for (int i = 0; i < NUM_VOICES; ++i) {
        if (!voices[i].noteHeld && voices[i].envVol < bestVol) {
            bestVol = voices[i].envVol;
            best = i;
        }
    }
    if (best >= 0) return best;

    // 3. Last resort: steal the quietest voice overall (least audible glitch)
    best = 0;
    bestVol = voices[0].envVol;
    for (int i = 1; i < NUM_VOICES; ++i) {
        if (voices[i].envVol < bestVol) {
            bestVol = voices[i].envVol;
            best = i;
        }
    }
    return best;
}

void CricketAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    for (int i = 0; i < NUM_VOICES; ++i)
        voices[i].params.sampleRate = sr;
}

void CricketAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalSamples = buffer.getNumSamples();
    buffer.clear();

    // Build params once per block — never inside the sample loop
    CricketVoice::Params currentParams;
    currentParams.sampleRate = currentSampleRate;
    fillVoiceParams(currentParams);

    // Handle MIDI — note on/off at block boundaries (acceptable granularity)
    for (const auto meta : midiMessages) {
        auto msg = meta.getMessage();
        if (msg.isNoteOn()) {
            for (int i = 0; i < NUM_VOICES; ++i)
                if (voiceNoteMap[i] == msg.getNoteNumber()) voices[i].noteOff();
            int v = allocateVoice();
            voices[v].params = currentParams;
            voices[v].noteOn(msg.getNoteNumber(), (float)msg.getVelocity() / 127.0f);
            voiceNoteMap[v] = msg.getNoteNumber();
        } else if (msg.isNoteOff()) {
            for (int i = 0; i < NUM_VOICES; ++i)
                if (voiceNoteMap[i] == msg.getNoteNumber()) {
                    voices[i].noteOff();
                    voiceNoteMap[i] = -1;
                }
        } else if (msg.isPitchWheel()) {
            currentPitchBendRaw = (msg.getPitchWheelValue() - 8192) / 8192.0f;
        } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
            for (int i = 0; i < NUM_VOICES; ++i) {
                voices[i].noteOff();
                voiceNoteMap[i] = -1;
            }
        }
    }

    // Update all active voices' params and phase increments ONCE per block
    for (int v = 0; v < NUM_VOICES; ++v) {
        if (voices[v].envState != ENV_END) {
            // Preserve noteHeld (it's on the voice, not in params, so this is safe)
            voices[v].params = currentParams;
            voices[v].computePhaseIncrements();
        }
    }

    float masterVol = *p_masterVol * 0.4f;
    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getWritePointer(1);

    for (int sample = 0; sample < totalSamples; ++sample) {
        float out = 0.0f;
        for (int v = 0; v < NUM_VOICES; ++v) {
            if (voices[v].envState != ENV_END)
                out += voices[v].tick();
        }
        out *= masterVol;
        // Simple hard clip — no tanh() in the audio thread
        out = std::fmax(-1.0f, std::fmin(1.0f, out));
        L[sample] = out;
        R[sample] = out;
    }
}

void CricketAudioProcessor::parameterChanged(const juce::String&, float) {}

void CricketAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void CricketAudioProcessor::setStateInformation(const void* data, int size)
{
    // In standalone mode, block the automatic startup restore (which would reload
    // the last session's state) but allow explicit user-initiated preset loads.
    if (wrapperType == wrapperType_Standalone && !allowStateRestore.load())
        return;

    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* CricketAudioProcessor::createEditor()
{
    return new CricketAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CricketAudioProcessor();
}
