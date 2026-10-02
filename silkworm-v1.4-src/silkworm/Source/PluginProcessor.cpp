#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
SilkwormAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Predelay: 0 – 500 ms, linear, default 0
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::predelay, "Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 500.0f, 0.1f), 0.0f));

    // Density: 0 – 1, default 0.5
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::density, "Density",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    // Character: 0 – 1, default 0.0 (clean)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::character_, "Character",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // Cutoff offset: bipolar -1..+1, default 0 (no shift)
    // Negative = darker than character setting, positive = brighter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::cutoffOfs, "Cutoff Offset",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    // Modulation amount: 0..1, default 0 (character controls mod depth)
    // Adds up to 12 samples of extra LFO modulation on delay read positions
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::modAmount, "Modulation",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // Decay: 0.1 – 45 s, skewed toward shorter values
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::decay, "Decay",
        juce::NormalisableRange<float>(0.1f, 45.0f, 0.01f, 0.3f), 2.0f));

    // Clock speed: 25 % – 100 %  (stored as 0.25 – 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::clockSpeed, "Clock Speed",
        juce::NormalisableRange<float>(0.25f, 1.0f, 0.01f), 1.0f));

    // Mix: 0 – 1, default 0.5
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::mix, "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    // Volume: 0 – 1, default 0.7
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::volume, "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    return { params.begin(), params.end() };
}

SilkwormAudioProcessor::SilkwormAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SilkwormState", createParameters())
{
    p_predelay   = apvts.getRawParameterValue(ParamID::predelay);
    p_density    = apvts.getRawParameterValue(ParamID::density);
    p_character  = apvts.getRawParameterValue(ParamID::character_);
    p_cutoffOfs  = apvts.getRawParameterValue(ParamID::cutoffOfs);
    p_modAmount  = apvts.getRawParameterValue(ParamID::modAmount);
    p_decay      = apvts.getRawParameterValue(ParamID::decay);
    p_clockSpeed = apvts.getRawParameterValue(ParamID::clockSpeed);
    p_mix        = apvts.getRawParameterValue(ParamID::mix);
    p_volume     = apvts.getRawParameterValue(ParamID::volume);
}

SilkwormAudioProcessor::~SilkwormAudioProcessor() {}

void SilkwormAudioProcessor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate);
}

void SilkwormAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    // Ensure stereo
    if (numCh < 2)
    {
        if (numCh == 1)
        {
            // Duplicate mono to a temp buffer for R
            buffer.setSize(2, numSamples, true, false, true);
            buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
        }
        else return;
    }

    // Build engine params from APVTS
    SilkwormEngine::Params ep;
    ep.predelayMs = *p_predelay;
    ep.density    = *p_density;
    ep.character    = *p_character;
    ep.cutoffOffset = *p_cutoffOfs;
    ep.modAmount    = *p_modAmount;
    ep.decaySec   = *p_decay;
    ep.clockSpeed = *p_clockSpeed;
    ep.mix        = *p_mix;
    ep.volume     = *p_volume;
    ep.sampleRate = getSampleRate();

    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getWritePointer(1);

    engine.process(L, R, L, R, numSamples, ep);
}

void SilkwormAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void SilkwormAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* SilkwormAudioProcessor::createEditor()
{
    return new SilkwormAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SilkwormAudioProcessor();
}
