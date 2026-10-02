#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
MothAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ---- Distortion (RAT) ----
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamID::distOn, "Distortion On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::distortion, "Distortion",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::filter, "Filter",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::distVolume, "Dist Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    // ---- Phaser (Krautrock) ----
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamID::phaserOn, "Phaser On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::speed, "Speed",
        juce::NormalisableRange<float>(0.02f, 30.0f, 0.001f, 0.30f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::depth, "Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::character, "Character",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::feedback, "Feedback",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParamID::lfoShape, "LFO Shape",
        juce::StringArray{"Sine","Triangle","Square","Sawtooth","Random"}, 0));

    // ---- Output ----
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::mix, "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::outLevel, "Output",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    return { params.begin(), params.end() };
}

MothAudioProcessor::MothAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input",   juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "MothState", createParameters())
{
    p_distOn     = apvts.getRawParameterValue(ParamID::distOn);
    p_distortion = apvts.getRawParameterValue(ParamID::distortion);
    p_filter     = apvts.getRawParameterValue(ParamID::filter);
    p_distVolume = apvts.getRawParameterValue(ParamID::distVolume);
    p_phaserOn   = apvts.getRawParameterValue(ParamID::phaserOn);
    p_speed      = apvts.getRawParameterValue(ParamID::speed);
    p_depth      = apvts.getRawParameterValue(ParamID::depth);
    p_character  = apvts.getRawParameterValue(ParamID::character);
    p_feedback   = apvts.getRawParameterValue(ParamID::feedback);
    p_lfoShape   = apvts.getRawParameterValue(ParamID::lfoShape);
    p_mix        = apvts.getRawParameterValue(ParamID::mix);
    p_outLevel   = apvts.getRawParameterValue(ParamID::outLevel);
}

MothAudioProcessor::~MothAudioProcessor() {}

bool MothAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono and stereo, in == out
    const auto& mainIn  = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();
    if (mainIn != mainOut) return false;
    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void MothAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    ratL.prepare(sr);
    ratR.prepare(sr);
    phaser.prepare(sr);
    mixSm.setTimeConstant(20.0f, sr);
    outSm.setTimeConstant(20.0f, sr);
    mixSm.setImmediate(*p_mix);
    outSm.setImmediate(*p_outLevel);
}

void MothAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const bool distOn   = (*p_distOn   > 0.5f);
    const bool phaserOn = (*p_phaserOn > 0.5f);

    // Update section params once per block
    ratL.setParams(*p_distortion, *p_filter, *p_distVolume);
    ratR.setParams(*p_distortion, *p_filter, *p_distVolume);
    phaser.setParams(*p_speed, *p_depth, *p_character, *p_feedback,
                     (int)*p_lfoShape);

    mixSm.target = *p_mix;
    outSm.target = *p_outLevel;

    auto* L = buffer.getWritePointer(0);
    auto* R = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    float meterL = 0.0f, meterR = 0.0f;

    for (int n = 0; n < numSamples; ++n) {
        float dryL = L[n];
        float dryR = R ? R[n] : dryL;

        float wetL = dryL;
        float wetR = dryR;

        // 1. Distortion (per channel)
        if (distOn) {
            wetL = ratL.processSample(wetL);
            wetR = ratR.processSample(wetR);
        }

        // 2. Phaser (stereo)
        if (phaserOn) {
            phaser.processStereo(wetL, wetR);
        }

        // Mix dry/wet
        float m = mixSm.next();
        float oL = dryL * (1.0f - m) + wetL * m;
        float oR = dryR * (1.0f - m) + wetR * m;

        // Output level
        float o = outSm.next();
        oL *= o;
        oR *= o;

        // Safety clip
        oL = moth::clampf(oL, -1.5f, 1.5f);
        oR = moth::clampf(oR, -1.5f, 1.5f);

        L[n] = oL;
        if (R) R[n] = oR;

        meterL = std::max(meterL, std::abs(oL));
        meterR = std::max(meterR, std::abs(oR));
    }

    // If mono, mirror meter
    if (!R) meterR = meterL;

    // Smooth meter decay handled in editor; just publish peak here
    outMeterL.store(meterL);
    outMeterR.store(meterR);
}

void MothAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void MothAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* MothAudioProcessor::createEditor()
{
    return new MothAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MothAudioProcessor();
}
