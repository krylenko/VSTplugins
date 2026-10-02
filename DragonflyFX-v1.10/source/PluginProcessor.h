#pragma once
#include <JuceHeader.h>
#include "FXEngine.h"

#define DRAGONFLY_VERSION "1.10"

// State version: bumped to 2 in v1.8 when the per-slot Bypass checkbox
// replaced the "Bypass" entry in the fxType choice list. States without
// a "stateVersion" attribute are v1 and get migrated on load.
#define DRAGONFLY_STATE_VERSION 2

// ============================================================
//  Parameter ID namespace
// ============================================================
namespace ParamID {
    inline juce::String fxType   (int b) { return "fxType"   + juce::String(b); }
    inline juce::String fxBypass (int b) { return "fxBypass" + juce::String(b); }
    inline juce::String fxParam0 (int b) { return "fxParam0" + juce::String(b); }
    inline juce::String fxParam1 (int b) { return "fxParam1" + juce::String(b); }

    // Global wet/dry mix
    inline const juce::String mix = "mix";

    inline juce::String lfoWave  (int l) { return "lfoWave"  + juce::String(l); }
    inline juce::String lfoSpeed (int l) { return "lfoSpeed" + juce::String(l); }
    inline juce::String lfoTarget(int l, int s) {
        return "lfoTarget" + juce::String(l) + "_" + juce::String(s); }
    inline juce::String lfoDepth (int l, int s) {
        return "lfoDepth"  + juce::String(l) + "_" + juce::String(s); }

    // Envelope follower
    inline const juce::String envSens  = "envSens";
    inline const juce::String envSpeed = "envSpeed";
    inline juce::String envTarget(int s) { return "envTarget_" + juce::String(s); }
    inline juce::String envDepth (int s) { return "envDepth_"  + juce::String(s); }
}

static constexpr int NUM_FX_BLOCKS     = 4;
static constexpr int NUM_LFOS          = 2;
static constexpr int LFO_DEPTH_SLOTS   = 2;
static constexpr int ENV_DEPTH_SLOTS   = 2;

// ============================================================
//  Dynamic modulation target name helper
//
//  Returns the 9-item StringArray (8 block params + None) used
//  to populate LFO and Envelope target combo boxes, with param
//  names reflecting each block's selected effect type.
//
//  blockTypes holds fxType CHOICE indices (0=MS20 LP .. 3=Chebyshev);
//  since v1.8 Bypass is a separate toggle, not a type.
//
//  Block prefixes: A, B, C, D
// ============================================================
static inline juce::StringArray buildModTargetNames(const int blockTypes[NUM_FX_BLOCKS])
{
    struct Labels { const char* p0; const char* p1; };
    static const Labels typeLabels[] = {
        { "Cutoff",   "Res"      },   // MS20 LP
        { "Cutoff",   "Res"      },   // MS20 HP
        { "Freq",     "Feedback" },   // Comb
        { "Drive",    "Order"    },   // Chebyshev
    };

    static const char blockLetter[] = { 'A', 'B', 'C', 'D' };

    juce::StringArray names;
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        int t = std::max(0, std::min(3, blockTypes[b]));
        juce::String prefix = juce::String::charToString(blockLetter[b]) + " ";
        names.add(prefix + typeLabels[t].p0);
        names.add(prefix + typeLabels[t].p1);
    }
    names.add("None");
    return names;
}

// ============================================================
//  DragonflyFXAudioProcessor
// ============================================================
class DragonflyFXAudioProcessor : public juce::AudioProcessor
{
public:
    DragonflyFXAudioProcessor();
    ~DragonflyFXAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Dragonfly FX"; }
    bool  acceptsMidi()  const override { return false; }
    bool  producesMidi() const override { return false; }
    bool  isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int  getNumPrograms()                             override { return 1; }
    int  getCurrentProgram()                          override { return 0; }
    void setCurrentProgram (int)                      override {}
    const juce::String getProgramName (int)           override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& dest) override;
    void setStateInformation (const void* data, int sz) override;

    juce::AudioProcessorValueTreeState apvts;

    // FX block params
    std::atomic<float>* p_fxType   [NUM_FX_BLOCKS] = {};
    std::atomic<float>* p_fxBypass [NUM_FX_BLOCKS] = {};
    std::atomic<float>* p_fxParam0 [NUM_FX_BLOCKS] = {};
    std::atomic<float>* p_fxParam1 [NUM_FX_BLOCKS] = {};

    // Global wet/dry mix
    std::atomic<float>* p_mix = nullptr;

    // LFO params
    std::atomic<float>* p_lfoWave  [NUM_LFOS] = {};
    std::atomic<float>* p_lfoSpeed [NUM_LFOS] = {};
    std::atomic<float>* p_lfoTarget[NUM_LFOS][LFO_DEPTH_SLOTS] = {};
    std::atomic<float>* p_lfoDepth [NUM_LFOS][LFO_DEPTH_SLOTS] = {};

    // Envelope follower params
    std::atomic<float>* p_envSens   = nullptr;
    std::atomic<float>* p_envSpeed  = nullptr;
    std::atomic<float>* p_envTarget[ENV_DEPTH_SLOTS] = {};
    std::atomic<float>* p_envDepth [ENV_DEPTH_SLOTS] = {};

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    FXBlock         fxBlocksL[NUM_FX_BLOCKS];
    FXBlock         fxBlocksR[NUM_FX_BLOCKS];
    LFOUnit         lfos[NUM_LFOS];
    EnvFollower     envFollower;
    DFSmoothedValue mixSm;
    double          currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragonflyFXAudioProcessor)
};
