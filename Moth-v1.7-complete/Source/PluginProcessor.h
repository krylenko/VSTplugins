#pragma once
#include <JuceHeader.h>
#include "MothEngine.h"

namespace ParamID {
    // Distortion section (RAT)
    inline const juce::String distOn      = "distOn";
    inline const juce::String distortion  = "distortion";
    inline const juce::String filter      = "filter";
    inline const juce::String distVolume  = "distVolume";

    // Phaser section (Krautrock)
    inline const juce::String phaserOn    = "phaserOn";
    inline const juce::String speed       = "speed";
    inline const juce::String depth       = "depth";
    inline const juce::String character   = "character";
    inline const juce::String feedback    = "feedback";
    inline const juce::String lfoShape    = "lfoShape";

    // Output
    inline const juce::String mix         = "mix";
    inline const juce::String outLevel    = "outLevel";
}

class MothAudioProcessor : public juce::AudioProcessor
{
public:
    MothAudioProcessor();
    ~MothAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Moth"; }
    bool  acceptsMidi()  const override { return false; }
    bool  producesMidi() const override { return false; }
    bool  isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms()                              override { return 1; }
    int  getCurrentProgram()                           override { return 0; }
    void setCurrentProgram(int)                        override {}
    const juce::String getProgramName(int)             override { return "Default"; }
    void changeProgramName(int, const juce::String&)   override {}

    void getStateInformation(juce::MemoryBlock& dest)  override;
    void setStateInformation(const void* data, int sz) override;

    juce::AudioProcessorValueTreeState apvts;

    // Metering for the UI (peak of the wet output, atomic, -inf..0 normalised 0..1)
    std::atomic<float> outMeterL { 0.0f };
    std::atomic<float> outMeterR { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    moth::RatDistortion ratL, ratR;
    moth::KrautPhaser   phaser;

    moth::SmoothedValue mixSm, outSm;

    std::atomic<float>* p_distOn      = nullptr;
    std::atomic<float>* p_distortion  = nullptr;
    std::atomic<float>* p_filter      = nullptr;
    std::atomic<float>* p_distVolume  = nullptr;
    std::atomic<float>* p_phaserOn    = nullptr;
    std::atomic<float>* p_speed       = nullptr;
    std::atomic<float>* p_depth       = nullptr;
    std::atomic<float>* p_character   = nullptr;
    std::atomic<float>* p_feedback    = nullptr;
    std::atomic<float>* p_lfoShape    = nullptr;
    std::atomic<float>* p_mix         = nullptr;
    std::atomic<float>* p_outLevel    = nullptr;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MothAudioProcessor)
};
