#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
DragonflyFXAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        // v1.8: "Bypass" removed from the type list — it is now a separate
        // per-slot toggle. Choice index 0..3 maps to FXType 1..4.
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            ParamID::fxType(b),
            "Block " + juce::String(b+1) + " Type",
            juce::StringArray{"MS20 LP","MS20 HP","Comb","Chebyshev"}, 0));

        // Default ON so a fresh instance passes audio through untouched,
        // matching the old all-slots-Bypass default.
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            ParamID::fxBypass(b),
            "Block " + juce::String(b+1) + " Bypass", true));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParamID::fxParam0(b),
            "Block " + juce::String(b+1) + " Cutoff",
            juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParamID::fxParam1(b),
            "Block " + juce::String(b+1) + " Resonance",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
        // (v1.10: the per-block DC Block toggle was removed — DC blocking is
        // now automatic for Chebyshev and MS-20 LP, see FXEngine.h.)
    }

    for (int l = 0; l < NUM_LFOS; ++l)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            ParamID::lfoWave(l),
            "LFO " + juce::String(l+1) + " Wave",
            juce::StringArray{"Sawtooth","Triangle","Noise","Square"}, 1));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParamID::lfoSpeed(l),
            "LFO " + juce::String(l+1) + " Speed",
            juce::NormalisableRange<float>(0.01f, 1000.0f, 0.001f, 0.3f), 1.0f));

        for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
        {
            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                ParamID::lfoTarget(l, s),
                "LFO " + juce::String(l+1) + " Target " + juce::String(s+1),
                juce::StringArray{
                    "A P1","A P2","B P1","B P2",
                    "C P1","C P2","D P1","D P2","None"
                },
                TARG_NONE));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                ParamID::lfoDepth(l, s),
                "LFO " + juce::String(l+1) + " Depth " + juce::String(s+1),
                juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
        }
    }

    // Envelope follower
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::envSens, "Env Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::envSpeed, "Env Speed",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            ParamID::envTarget(s),
            "Env Target " + juce::String(s+1),
            juce::StringArray{
                "A P1","A P2","B P1","B P2",
                "C P1","C P2","D P1","D P2","None"
            },
            TARG_NONE));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParamID::envDepth(s),
            "Env Depth " + juce::String(s+1),
            juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    }

    // Global wet/dry mix. Default 1.0 (fully wet) preserves existing behaviour.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::mix, "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));

    return { params.begin(), params.end() };
}

DragonflyFXAudioProcessor::DragonflyFXAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "DragonflyFXState", createParameters())
{
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        p_fxType   [b] = apvts.getRawParameterValue(ParamID::fxType(b));
        p_fxBypass [b] = apvts.getRawParameterValue(ParamID::fxBypass(b));
        p_fxParam0 [b] = apvts.getRawParameterValue(ParamID::fxParam0(b));
        p_fxParam1 [b] = apvts.getRawParameterValue(ParamID::fxParam1(b));
    }
    p_mix = apvts.getRawParameterValue(ParamID::mix);
    for (int l = 0; l < NUM_LFOS; ++l)
    {
        p_lfoWave [l] = apvts.getRawParameterValue(ParamID::lfoWave(l));
        p_lfoSpeed[l] = apvts.getRawParameterValue(ParamID::lfoSpeed(l));
        for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
        {
            p_lfoTarget[l][s] = apvts.getRawParameterValue(ParamID::lfoTarget(l, s));
            p_lfoDepth [l][s] = apvts.getRawParameterValue(ParamID::lfoDepth(l, s));
        }
    }
    p_envSens  = apvts.getRawParameterValue(ParamID::envSens);
    p_envSpeed = apvts.getRawParameterValue(ParamID::envSpeed);
    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
    {
        p_envTarget[s] = apvts.getRawParameterValue(ParamID::envTarget(s));
        p_envDepth [s] = apvts.getRawParameterValue(ParamID::envDepth(s));
    }
}

DragonflyFXAudioProcessor::~DragonflyFXAudioProcessor() {}

void DragonflyFXAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        fxBlocksL[b].sampleRate = sr;
        fxBlocksR[b].sampleRate = sr;
        fxBlocksL[b].reset();
        fxBlocksR[b].reset();
    }
    for (int l = 0; l < NUM_LFOS; ++l)
        lfos[l].sampleRate = sr;
    envFollower.sampleRate = sr;

    mixSm.setTimeConstant(20.0f, sr);
    mixSm.setImmediate(*p_mix);
}

void DragonflyFXAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Read block params once per block
    float baseParam0[NUM_FX_BLOCKS], baseParam1[NUM_FX_BLOCKS];
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        // Bypass toggle wins: engine type FX_BYPASS passes audio through
        // unchanged. Otherwise map choice index 0..3 -> FXType 1..4.
        const bool bypassed = *p_fxBypass[b] > 0.5f;
        fxBlocksL[b].type = fxBlocksR[b].type =
            bypassed ? FX_BYPASS : (FXType)((int)*p_fxType[b] + 1);
        baseParam0[b] = *p_fxParam0[b];
        baseParam1[b] = *p_fxParam1[b];
        fxBlocksL[b].param0 = fxBlocksR[b].param0 = baseParam0[b];
        fxBlocksL[b].param1 = fxBlocksR[b].param1 = baseParam1[b];
    }
    for (int l = 0; l < NUM_LFOS; ++l)
    {
        lfos[l].wave    = (LFOWave)(int)*p_lfoWave[l];
        lfos[l].speedHz = *p_lfoSpeed[l];
    }
    envFollower.sensitivity = *p_envSens;
    envFollower.speed       = *p_envSpeed;

    mixSm.target = *p_mix;

    auto* L = numChannels > 0 ? buffer.getWritePointer(0) : nullptr;
    auto* R = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Accumulate modulation into effective params
        float effP0[NUM_FX_BLOCKS], effP1[NUM_FX_BLOCKS];
        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        {
            effP0[b] = baseParam0[b];
            effP1[b] = baseParam1[b];
        }

        // LFO modulation
        for (int l = 0; l < NUM_LFOS; ++l)
        {
            lfos[l].tick();
            float lfoVal = lfos[l].value;

            for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
            {
                int   target = (int)*p_lfoTarget[l][s];
                float depth  = *p_lfoDepth[l][s];
                if (target == TARG_NONE) continue;
                int   block  = target / 2;
                int   pIdx   = target % 2;
                float mod    = lfoVal * depth;
                if (pIdx == 0) effP0[block] += mod;
                else           effP1[block] += mod;
            }
        }

        // Envelope follower modulation
        // Feed the mono sum of the PRE-FX input to the envelope detector
        float envInput = 0.0f;
        if (L != nullptr) envInput += L[sample];
        if (R != nullptr) envInput += R[sample];
        envInput *= 0.5f;
        envFollower.tick(envInput);
        float envVal = envFollower.envelope;  // 0..1

        for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
        {
            int   target = (int)*p_envTarget[s];
            float depth  = *p_envDepth[s];
            if (target == TARG_NONE) continue;
            int   block  = target / 2;
            int   pIdx   = target % 2;
            float mod    = envVal * depth;
            if (pIdx == 0) effP0[block] += mod;
            else           effP1[block] += mod;
        }

        // Push effective values to blocks
        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        {
            fxBlocksL[b].eff_param0 = effP0[b];
            fxBlocksL[b].eff_param1 = effP1[b];
            fxBlocksR[b].eff_param0 = effP0[b];
            fxBlocksR[b].eff_param1 = effP1[b];
        }

        float m = mixSm.next();

        if (L != nullptr) {
            float dry = L[sample];
            float sig = dry;
            for (int b = 0; b < NUM_FX_BLOCKS; ++b) sig = fxBlocksL[b].process(sig);
            L[sample] = dry * (1.0f - m) + sig * m;
        }
        if (R != nullptr) {
            float dry = R[sample];
            float sig = dry;
            for (int b = 0; b < NUM_FX_BLOCKS; ++b) sig = fxBlocksR[b].process(sig);
            R[sample] = dry * (1.0f - m) + sig * m;
        }
    }
}

void DragonflyFXAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute("stateVersion", DRAGONFLY_STATE_VERSION);
    copyXmlToBinary(*xml, dest);
}

void DragonflyFXAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml == nullptr || !xml->hasTagName(apvts.state.getType()))
        return;

    // ---- v1 -> v2 state migration (v1.7 and earlier) ----
    // Old states stored fxType 0..4 where 0 meant "Bypass". Since v1.8
    // Bypass is its own bool param and fxType is 0..3 (MS20 LP..Chebyshev).
    if (xml->getIntAttribute("stateVersion", 1) < 2)
    {
        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        {
            const juce::String typeId   = ParamID::fxType(b);
            const juce::String bypassId = ParamID::fxBypass(b);

            // Find this block's fxType PARAM child (if absent, the block
            // was at default = Bypass; treat it as such).
            juce::XmlElement* typeEl = nullptr;
            for (auto* e : xml->getChildIterator())
                if (e->hasTagName("PARAM") &&
                    e->getStringAttribute("id") == typeId) { typeEl = e; break; }

            const float oldVal = typeEl != nullptr
                ? (float)typeEl->getDoubleAttribute("value", 0.0) : 0.0f;
            const bool wasBypass = oldVal < 0.5f;

            // Rewrite fxType to the new 0..3 choice index.
            if (typeEl == nullptr)
            {
                typeEl = xml->createNewChildElement("PARAM");
                typeEl->setAttribute("id", typeId);
            }
            typeEl->setAttribute("value",
                wasBypass ? 0.0 : (double)(oldVal - 1.0f));

            // Add the bypass param (explicitly, for every block — the new
            // param defaults to ON, so active blocks must write OFF).
            auto* bypEl = xml->createNewChildElement("PARAM");
            bypEl->setAttribute("id", bypassId);
            bypEl->setAttribute("value", wasBypass ? 1.0 : 0.0);
        }
    }

    apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* DragonflyFXAudioProcessor::createEditor()
{
    return new DragonflyFXAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DragonflyFXAudioProcessor();
}
