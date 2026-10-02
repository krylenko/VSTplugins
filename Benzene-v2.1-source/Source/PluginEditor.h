#pragma once
#include <JuceHeader.h>
#include <array>
#include "PluginProcessor.h"

// ============================================================
//  BenzeneLookAndFeel — 1950s lab-instrument styling
// ============================================================
class BenzeneLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BenzeneLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float startAng, float endAng,
                           juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int,
                       juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;
};

// ============================================================
//  LabelledKnob
// ============================================================
class LabelledKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;
    LabelledKnob (const juce::String& name, juce::Colour col);
    void resized() override;
    void setValueSuffix (const juce::String& s);
private:
    juce::Colour ring;
};

// ============================================================
//  MiniKnob — compact knob for the modulation matrix (label below, small)
// ============================================================
class MiniKnob : public juce::Component
{
public:
    juce::Slider slider;
    MiniKnob (juce::Colour col);
    void resized() override;
};

// ============================================================
//  Editor
// ============================================================
class BenzeneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    BenzeneAudioProcessorEditor (BenzeneAudioProcessor&);
    ~BenzeneAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    BenzeneAudioProcessor& processor;
    BenzeneLookAndFeel laf;

    // Signal Generator panel
    juce::ComboBox carrierWave;
    LabelledKnob   knobFreq   { "FREQUENCY", juce::Colour (0xFFE8A33D) };
    LabelledKnob   knobDrive  { "DRIVE",     juce::Colour (0xFFE8A33D) };
    LabelledKnob   knobDrift  { "DRIFT",     juce::Colour (0xFFE8A33D) };
    LabelledKnob   knobSym    { "SYMMETRY",  juce::Colour (0xFFE8A33D) };

    // Ring Modulator panel
    juce::ComboBox diodeType;
    juce::ComboBox stability;
    LabelledKnob   knobImbal   { "IMBALANCE",   juce::Colour (0xFF7FB3C9) };
    LabelledKnob   knobInstab  { "INSTABILITY", juce::Colour (0xFFC97F7F) };
    LabelledKnob   knobInLvl   { "INPUT",       juce::Colour (0xFF7FB3C9) };
    LabelledKnob   knobCarLvl  { "CARRIER",     juce::Colour (0xFF7FB3C9) };

    // Output panel
    LabelledKnob   knobMix     { "MIX",    juce::Colour (0xFFB7B7A8) };
    LabelledKnob   knobOut     { "OUTPUT", juce::Colour (0xFFB7B7A8) };

    juce::ToggleButton gateButton { "GATE" };

    juce::Label freqReadout;

    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SAtt> aFreq, aDrive, aDrift, aSym, aImbal, aInstab, aInLvl, aCarLvl, aMix, aOut;
    std::unique_ptr<CAtt> aWave, aDiode, aStab;
    std::unique_ptr<BAtt> aGate;

    // ---- modulation strips ----
    // Source controls
    LabelledKnob lfoRate  { "RATE",  juce::Colour (0xFF9C7FC9) };
    LabelledKnob lfoBlend { "SHAPE", juce::Colour (0xFF9C7FC9) };
    LabelledKnob pitchSens{ "SENS",  juce::Colour (0xFF7FC99C) };
    LabelledKnob envSens  { "SENS",  juce::Colour (0xFFC9B47F) };
    LabelledKnob envSpeed { "SPEED", juce::Colour (0xFFC9B47F) };

    // 3 sources x 8 destinations depth knobs
    std::array<std::array<std::unique_ptr<MiniKnob>, 8>, 3> depth;

    std::unique_ptr<SAtt> aLfoRate, aLfoBlend, aPitchSens, aEnvSens, aEnvSpeed;
    std::array<std::array<std::unique_ptr<SAtt>, 8>, 3> aDepth;

    // live source activity indicators (lit from processor display atomics)
    float lfoLevel = 0.0f, pitchLevel = 0.0f, envLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BenzeneAudioProcessorEditor)
};
