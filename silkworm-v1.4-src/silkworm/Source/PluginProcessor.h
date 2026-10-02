#pragma once
#include <JuceHeader.h>
#include "SilkwormEngine.h"

namespace ParamID {
    inline const juce::String predelay    = "predelay";
    inline const juce::String density     = "density";
    inline const juce::String character_  = "character";  // trailing _ avoids keyword clash
    inline const juce::String cutoffOfs  = "cutoffOfs";
    inline const juce::String modAmount  = "modAmount";
    inline const juce::String decay       = "decay";
    inline const juce::String clockSpeed  = "clockSpeed";
    inline const juce::String mix         = "mix";
    inline const juce::String volume      = "volume";
}

class SilkwormAudioProcessor : public juce::AudioProcessor
{
public:
    SilkwormAudioProcessor();
    ~SilkwormAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Silkworm"; }
    bool  acceptsMidi()  const override { return false; }
    bool  producesMidi() const override { return false; }
    bool  isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 45.0; }

    int  getNumPrograms()                              override { return 1; }
    int  getCurrentProgram()                           override { return 0; }
    void setCurrentProgram(int)                        override {}
    const juce::String getProgramName(int)             override { return "Default"; }
    void changeProgramName(int, const juce::String&)   override {}

    void getStateInformation(juce::MemoryBlock& dest)  override;
    void setStateInformation(const void* data, int sz) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    SilkwormEngine engine;

    std::atomic<float>* p_predelay   = nullptr;
    std::atomic<float>* p_density    = nullptr;
    std::atomic<float>* p_character  = nullptr;
    std::atomic<float>* p_cutoffOfs  = nullptr;
    std::atomic<float>* p_modAmount  = nullptr;
    std::atomic<float>* p_decay      = nullptr;
    std::atomic<float>* p_clockSpeed = nullptr;
    std::atomic<float>* p_mix        = nullptr;
    std::atomic<float>* p_volume     = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilkwormAudioProcessor)
};
