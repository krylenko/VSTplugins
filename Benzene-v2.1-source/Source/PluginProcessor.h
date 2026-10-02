#pragma once
#include <JuceHeader.h>
#include "BenzeneEngine.h"
#include "ModEngine.h"

namespace ParamID {
    inline const juce::String carrierFreq  = "carrierFreq";
    inline const juce::String carrierWave  = "carrierWave";
    inline const juce::String carrierDrive = "carrierDrive";
    inline const juce::String carrierDrift = "carrierDrift";
    inline const juce::String symmetry     = "symmetry";
    inline const juce::String diodeType    = "diodeType";
    inline const juce::String imbalance    = "imbalance";
    inline const juce::String instability  = "instability";
    inline const juce::String inputLevel   = "inputLevel";
    inline const juce::String carrierLevel = "carrierLevel";
    inline const juce::String oversample   = "oversample";
    inline const juce::String mix          = "mix";
    inline const juce::String outputLevel  = "outputLevel";
    inline const juce::String gateEnable   = "gateEnable";

    // --- modulation source controls ---
    inline const juce::String lfoRate      = "lfoRate";
    inline const juce::String lfoBlend     = "lfoBlend";
    inline const juce::String pitchSens    = "pitchSens";
    inline const juce::String envSens      = "envSens";
    inline const juce::String envSpeed     = "envSpeed";

    // --- modulation matrix ---
    // The 8 continuous destinations, in a fixed order used everywhere.
    static const juce::StringArray modDestIDs {
        "carrierFreq", "carrierDrive", "carrierDrift", "symmetry",
        "imbalance", "instability", "inputLevel", "carrierLevel"
    };
    static const juce::StringArray modDestLabels {
        "FREQ", "DRIVE", "DRIFT", "SYM",
        "IMBAL", "INST", "INPUT", "CARR"
    };
    // The 3 sources, in fixed order.
    static const juce::StringArray modSrcIDs    { "lfo", "pitch", "env" };
    static const juce::StringArray modSrcLabels { "LFO", "PITCH", "ENV" };

    // depth param id, e.g. "mod_lfo_carrierFreq"
    inline juce::String modDepth (int src, int dest) {
        return "mod_" + modSrcIDs[src] + "_" + modDestIDs[dest];
    }
}

class BenzeneAudioProcessor : public juce::AudioProcessor
{
public:
    BenzeneAudioProcessor();
    ~BenzeneAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override { return true; }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Benzene"; }
    bool  acceptsMidi()  const override { return false; }
    bool  producesMidi() const override { return false; }
    bool  isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms() override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // for the editor's carrier-frequency readout
    std::atomic<float> displayCarrierHz { 440.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    bz::CarrierOscillator  carrierL, carrierR;
    bz::DiodeRing ringL, ringR;

    // modulation sources
    bz::LFOSource          lfo;
    bz::PitchTrackerSource pitchTracker;
    bz::EnvFollowerSource  envFollower;

    std::atomic<float>* p_carrierFreq  = nullptr;
    std::atomic<float>* p_carrierWave  = nullptr;
    std::atomic<float>* p_carrierDrive = nullptr;
    std::atomic<float>* p_carrierDrift = nullptr;
    std::atomic<float>* p_symmetry     = nullptr;
    std::atomic<float>* p_diodeType    = nullptr;
    std::atomic<float>* p_imbalance    = nullptr;
    std::atomic<float>* p_instability  = nullptr;
    std::atomic<float>* p_inputLevel   = nullptr;
    std::atomic<float>* p_carrierLevel = nullptr;
    std::atomic<float>* p_oversample   = nullptr;
    std::atomic<float>* p_mix          = nullptr;
    std::atomic<float>* p_outputLevel  = nullptr;
    std::atomic<float>* p_gateEnable   = nullptr;

    // modulation source control pointers
    std::atomic<float>* p_lfoRate   = nullptr;
    std::atomic<float>* p_lfoBlend  = nullptr;
    std::atomic<float>* p_pitchSens = nullptr;
    std::atomic<float>* p_envSens   = nullptr;
    std::atomic<float>* p_envSpeed  = nullptr;

    // modulation depth matrix: [source 0..2][dest 0..7]
    std::atomic<float>* p_modDepth[3][8] = {};

    // for the editor to display live source values
public:
    std::atomic<float> dispLFO { 0.0f }, dispPitch { 0.0f }, dispEnv { 0.0f };
    std::atomic<float> dispDepth[3][8] = {};
private:

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BenzeneAudioProcessor)
};
