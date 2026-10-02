#pragma once
#include <JuceHeader.h>
#include "CricketEngine.h"

static constexpr int NUM_VOICES = 8;

namespace ParamID {
    inline const juce::String osc1Wave    = "osc1Wave";
    inline const juce::String osc2Wave    = "osc2Wave";
    inline const juce::String lfoWave     = "lfoWave";
    // Pitch: semitone offset -24..+24, 0 = MIDI-accurate
    inline const juce::String osc1Pitch   = "osc1Pitch";
    inline const juce::String osc2Pitch   = "osc2Pitch";
    inline const juce::String oscMix      = "oscMix";
    inline const juce::String attack      = "attack";
    inline const juce::String decay       = "decay";
    inline const juce::String sustain     = "sustain";
    inline const juce::String release_    = "release";
    inline const juce::String fmDepth     = "fmDepth";
    inline const juce::String lfoSpeed    = "lfoSpeed";
    inline const juce::String lfoDepth    = "lfoDepth";
    inline const juce::String wrap        = "wrap";
    inline const juce::String logicOn     = "logicOn";
    inline const juce::String logicMode   = "logicMode";
    inline const juce::String osc2Range   = "osc2Range";
    inline const juce::String velSensitivity = "velSensitivity";
    inline const juce::String osc1BendRange  = "osc1BendRange";
    inline const juce::String osc2BendRange  = "osc2BendRange";
    inline const juce::String modAttack      = "modAttack";
    inline const juce::String modDecay       = "modDecay";
    inline const juce::String modSustain     = "modSustain";
    inline const juce::String modRelease_    = "modRelease";
    inline const juce::String modLoop        = "modLoop";
    inline const juce::String modToOsc1Pitch = "modToOsc1Pitch";
    inline const juce::String modToOsc2Pitch = "modToOsc2Pitch";
    inline const juce::String modToFMDepth   = "modToFMDepth";
    inline const juce::String modToLFOSpeed  = "modToLFOSpeed";
    inline const juce::String modToWrap      = "modToWrap";
    inline const juce::String modToCutoff    = "modToCutoff";
    inline const juce::String filterType     = "filterType";
    inline const juce::String filterCutoff   = "filterCutoff";
    inline const juce::String filterRes      = "filterRes";
    inline const juce::String lfoToCutoff    = "lfoToCutoff";
    inline const juce::String lfoToRes       = "lfoToRes";
    inline const juce::String masterVol      = "masterVol";
}

class CricketAudioProcessor : public juce::AudioProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
public:
    CricketAudioProcessor();
    ~CricketAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Cricket"; }
    bool  acceptsMidi()  const override { return true; }
    bool  producesMidi() const override { return false; }
    bool  isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int  getNumPrograms()                              override { return 1; }
    int  getCurrentProgram()                           override { return 0; }
    void setCurrentProgram(int)                        override {}
    const juce::String getProgramName(int)             override { return "Default"; }
    void changeProgramName(int, const juce::String&)   override {}

    void getStateInformation(juce::MemoryBlock& dest)  override;
    void setStateInformation(const void* data, int sz) override;
    void parameterChanged(const juce::String& id, float val) override;

    juce::AudioProcessorValueTreeState apvts;

    // Public state accessible by standalone app and editor
    std::atomic<bool> allowStateRestore { false };
    juce::String      presetName;        // filename without extension, shown in title bar
    juce::File        lastPresetFolder;  // remembered across loads within a session

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    CricketVoice voices[NUM_VOICES];
    int          voiceNoteMap[NUM_VOICES];

    int  allocateVoice();
    void fillVoiceParams(CricketVoice::Params& p);

    std::atomic<float>* p_osc1Wave   = nullptr;
    std::atomic<float>* p_osc2Wave   = nullptr;
    std::atomic<float>* p_lfoWave    = nullptr;
    std::atomic<float>* p_osc1Pitch  = nullptr;
    std::atomic<float>* p_osc2Pitch  = nullptr;
    std::atomic<float>* p_oscMix     = nullptr;
    std::atomic<float>* p_attack     = nullptr;
    std::atomic<float>* p_decay      = nullptr;
    std::atomic<float>* p_sustain    = nullptr;
    std::atomic<float>* p_release    = nullptr;
    std::atomic<float>* p_fmDepth    = nullptr;
    std::atomic<float>* p_lfoSpeed   = nullptr;
    std::atomic<float>* p_lfoDepth   = nullptr;
    std::atomic<float>* p_wrap       = nullptr;
    std::atomic<float>* p_logicOn    = nullptr;
    std::atomic<float>* p_logicMode  = nullptr;
    std::atomic<float>* p_osc2Range  = nullptr;
    std::atomic<float>* p_masterVol  = nullptr;
    std::atomic<float>* p_velSensitivity = nullptr;
    std::atomic<float>* p_osc1BendRange  = nullptr;
    std::atomic<float>* p_osc2BendRange  = nullptr;
    std::atomic<float>* p_modAttack      = nullptr;
    std::atomic<float>* p_modDecay       = nullptr;
    std::atomic<float>* p_modSustain     = nullptr;
    std::atomic<float>* p_modRelease     = nullptr;
    std::atomic<float>* p_modLoop        = nullptr;
    std::atomic<float>* p_modToOsc1Pitch = nullptr;
    std::atomic<float>* p_modToOsc2Pitch = nullptr;
    std::atomic<float>* p_modToFMDepth   = nullptr;
    std::atomic<float>* p_modToLFOSpeed  = nullptr;
    std::atomic<float>* p_modToWrap      = nullptr;
    std::atomic<float>* p_modToCutoff    = nullptr;
    std::atomic<float>* p_filterType     = nullptr;
    std::atomic<float>* p_filterCutoff   = nullptr;
    std::atomic<float>* p_filterRes      = nullptr;
    std::atomic<float>* p_lfoToCutoff    = nullptr;
    std::atomic<float>* p_lfoToRes       = nullptr;

    float currentPitchBendRaw = 0.0f;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CricketAudioProcessor)
};
