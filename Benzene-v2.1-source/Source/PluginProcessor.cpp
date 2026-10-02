#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
BenzeneAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Carrier frequency: 0.01 Hz .. 100 kHz, heavily skewed so the musical
    // low/mid range gets most of the knob travel.
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::carrierFreq, "Carrier Freq",
        juce::NormalisableRange<float>(0.01f, 100000.0f, 0.0f, 0.18f), 440.0f));

    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        ParamID::carrierWave, "Carrier Wave",
        juce::StringArray{ "Sine", "Triangle", "Sawtooth", "Pulse",
                           "White", "Pink", "LFSR" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::carrierDrive, "Carrier Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::carrierDrift, "Carrier Drift",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::symmetry, "Symmetry",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        ParamID::diodeType, "Diode Type",
        juce::StringArray{ "Germanium", "Silicon" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::imbalance, "Imbalance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.15f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::instability, "Instability",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::inputLevel, "Input Level",
        juce::NormalisableRange<float>(0.0f, 2.0f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::carrierLevel, "Carrier Level",
        juce::NormalisableRange<float>(0.0f, 2.0f), 1.0f));

    // Oversampling: lower = grittier / more aliasing, higher = smoother.
    // All values are above the solver's stability floor (~24x), so none break up.
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        ParamID::oversample, "Stability",
        juce::StringArray{ "Raw (1x)", "Gritty (2x)", "Smooth (4x)", "Clean (8x)" }, 1));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::mix, "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::outputLevel, "Output",
        juce::NormalisableRange<float>(0.0f, 2.0f), 1.0f));

    // Gate on by default: carrier is silent when there's no input. Turn off to
    // use Benzene as a standalone sound source (carrier passes continuously).
    params.push_back (std::make_unique<juce::AudioParameterBool>(
        ParamID::gateEnable, "Input Gate", true));

    // ---- modulation source controls ----
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::lfoRate, "LFO Rate",
        juce::NormalisableRange<float>(0.02f, 40.0f, 0.0f, 0.35f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::lfoBlend, "LFO Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::pitchSens, "Pitch Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::envSens, "Env Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        ParamID::envSpeed, "Env Speed",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    // ---- modulation depth matrix (3 sources x 8 destinations) ----
    for (int s = 0; s < ParamID::modSrcIDs.size(); ++s)
        for (int d = 0; d < ParamID::modDestIDs.size(); ++d)
            params.push_back (std::make_unique<juce::AudioParameterFloat>(
                ParamID::modDepth (s, d),
                ParamID::modSrcLabels[s] + "->" + ParamID::modDestLabels[d],
                juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    return { params.begin(), params.end() };
}

BenzeneAudioProcessor::BenzeneAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "BenzeneState", createParameters())
{
    p_carrierFreq  = apvts.getRawParameterValue (ParamID::carrierFreq);
    p_carrierWave  = apvts.getRawParameterValue (ParamID::carrierWave);
    p_carrierDrive = apvts.getRawParameterValue (ParamID::carrierDrive);
    p_carrierDrift = apvts.getRawParameterValue (ParamID::carrierDrift);
    p_symmetry     = apvts.getRawParameterValue (ParamID::symmetry);
    p_diodeType    = apvts.getRawParameterValue (ParamID::diodeType);
    p_imbalance    = apvts.getRawParameterValue (ParamID::imbalance);
    p_instability  = apvts.getRawParameterValue (ParamID::instability);
    p_inputLevel   = apvts.getRawParameterValue (ParamID::inputLevel);
    p_carrierLevel = apvts.getRawParameterValue (ParamID::carrierLevel);
    p_oversample   = apvts.getRawParameterValue (ParamID::oversample);
    p_mix          = apvts.getRawParameterValue (ParamID::mix);
    p_outputLevel  = apvts.getRawParameterValue (ParamID::outputLevel);
    p_gateEnable   = apvts.getRawParameterValue (ParamID::gateEnable);

    p_lfoRate   = apvts.getRawParameterValue (ParamID::lfoRate);
    p_lfoBlend  = apvts.getRawParameterValue (ParamID::lfoBlend);
    p_pitchSens = apvts.getRawParameterValue (ParamID::pitchSens);
    p_envSens   = apvts.getRawParameterValue (ParamID::envSens);
    p_envSpeed  = apvts.getRawParameterValue (ParamID::envSpeed);

    for (int s = 0; s < 3; ++s)
        for (int d = 0; d < 8; ++d)
            p_modDepth[s][d] = apvts.getRawParameterValue (ParamID::modDepth (s, d));
}

BenzeneAudioProcessor::~BenzeneAudioProcessor() {}

void BenzeneAudioProcessor::prepareToPlay (double sr, int)
{
    currentSampleRate = sr;
    carrierL.prepare (sr);
    carrierR.prepare (sr);
    ringL.prepare (sr);
    ringR.prepare (sr);
    lfo.prepare (sr);
    pitchTracker.prepare (sr);
    envFollower.prepare (sr);
}

void BenzeneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const float baseFreq   = *p_carrierFreq;
    const auto  wave   = (bz::CarrierOscillator::Shape) (int) *p_carrierWave;
    const float baseDrive  = *p_carrierDrive;
    const float baseDrift  = *p_carrierDrift;
    const float baseSym    = *p_symmetry;
    const auto  diode  = (bz::DiodeRing::DiodeType) (int) *p_diodeType;
    const float baseImbal  = *p_imbalance;
    const float baseInstab = *p_instability;
    const float baseInLvl  = *p_inputLevel;
    const float baseCarLvl = *p_carrierLevel;
    const float mix    = *p_mix;
    const float outLvl = *p_outputLevel;
    const bool  gateOn = (*p_gateEnable > 0.5f);

    const int osTable[4] = { 1, 2, 4, 8 };
    const int os = osTable[ juce::jlimit (0, 3, (int) *p_oversample) ];

    // ---- compute modulation sources once per block ----
    lfo.setRateHz (*p_lfoRate);
    lfo.setBlend (*p_lfoBlend);
    pitchTracker.setSensitivity (*p_pitchSens);
    envFollower.setSensitivity (*p_envSens);
    envFollower.setSpeed (*p_envSpeed);

    const float* in0 = buffer.getReadPointer (0);
    float srcLFO   = lfo.process (numSamples);
    float srcPitch = pitchTracker.process (in0, numSamples);
    float srcEnv   = envFollower.process (in0, numSamples);
    lfo.store (srcLFO);

    dispLFO.store (srcLFO);
    dispPitch.store (srcPitch);
    dispEnv.store (srcEnv);

    const float srcVal[3] = { srcLFO, srcPitch, srcEnv };

    // Apply the modulation matrix to each destination. Each destination has a
    // characteristic full-scale span; bipolar depth * source scales that span.
    // dest order: 0 freq, 1 drive, 2 drift, 3 sym, 4 imbal, 5 instab, 6 inLvl, 7 carLvl
    float modSum[8] = {0,0,0,0,0,0,0,0};
    for (int d = 0; d < 8; ++d)
        for (int s = 0; s < 3; ++s)
        {
            float dep = p_modDepth[s][d]->load();
            modSum[d] += dep * srcVal[s];
            dispDepth[s][d].store (dep);
        }

    // Map the (already bipolar, ~-1..1 per unit depth) sums onto each parameter,
    // scaled to a musical span, then clamp to the parameter's valid range.
    // Frequency modulates multiplicatively (octaves) so it tracks musically.
    float freq = baseFreq * std::pow (2.0f, modSum[0] * 2.0f);   // +/-2 octaves per unit
    freq = juce::jlimit (0.01f, 100000.0f, freq);

    float drive  = juce::jlimit (0.0f, 1.0f, baseDrive  + modSum[1]);
    float drift  = juce::jlimit (0.0f, 1.0f, baseDrift  + modSum[2]);
    float sym    = juce::jlimit (-1.0f, 1.0f, baseSym   + modSum[3]);
    float imbal  = juce::jlimit (0.0f, 1.0f, baseImbal  + modSum[4]);
    float instab = juce::jlimit (0.0f, 1.0f, baseInstab + modSum[5]);
    float inLvl  = juce::jlimit (0.0f, 2.0f, baseInLvl  + modSum[6] * 2.0f);
    float carLvl = juce::jlimit (0.0f, 2.0f, baseCarLvl + modSum[7] * 2.0f);

    displayCarrierHz.store (freq);

    auto setup = [&] (bz::CarrierOscillator& c, bz::DiodeRing& r)
    {
        c.setFrequency (freq);
        c.setShape (wave);
        c.setDrive (drive);
        c.setDrift (drift);
        c.setSymmetry (sym);
        r.setDiode (diode);
        r.setImbalance (imbal);
        r.setInstability (instab);
        r.setGateEnabled (gateOn);
        r.setInputLevel (inLvl);
        r.setCarrierLevel (carLvl);
        r.setOversample (os);
    };
    setup (carrierL, ringL);
    setup (carrierR, ringR);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        auto& carrier = (ch == 0) ? carrierL : carrierR;
        auto& ring    = (ch == 0) ? ringL    : ringR;

        for (int i = 0; i < numSamples; ++i)
        {
            const float dry = data[i];
            const float c   = carrier.tick();
            float wet = ring.process (dry, c);

            if (! std::isfinite (wet)) { wet = 0.0f; ring.reset(); }

            data[i] = (dry * (1.0f - mix) + wet * mix) * outLvl;
        }
    }

    // If the host gave us a single carrier instance per channel but mono input
    // duplicated to stereo, the two carriers drift independently -> subtle width.
}

void BenzeneAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}

void BenzeneAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* BenzeneAudioProcessor::createEditor()
{
    return new BenzeneAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BenzeneAudioProcessor();
}
