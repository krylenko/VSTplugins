#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DragonflyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DragonflyLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAng, float endAng,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int w, int h,
                      bool down, int, int, int, int, juce::ComboBox&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
    juce::Font getLabelFont(juce::Label&) override;
};

class LabelledKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;
    juce::Colour ringColour;
    LabelledKnob(const juce::String& name, juce::Colour col);
    void resized() override;
};

// ============================================================
//  FXBlockPanel
// ============================================================
class FXBlockPanel : public juce::Component
{
public:
    int                blockIndex;
    juce::ComboBox     typeCombo;
    juce::ToggleButton bypassBtn { "BYPASS" };
    LabelledKnob       knobP0 { "", juce::Colour(0xFF66AAFF) };
    LabelledKnob       knobP1 { "", juce::Colour(0xFFFF88AA) };

    explicit FXBlockPanel(int idx);
    void paint(juce::Graphics&) override;
    void resized() override;
    void refreshKnobLabels(int typeChoice);   // 0=MS20 LP .. 3=Chebyshev
    void setBypassDim(bool bypassed);
};

// ============================================================
//  DepthSlot — shared between LFO and Env panels
// ============================================================
struct DepthSlot {
    juce::Label    slotLabel;
    juce::ComboBox targetCombo;
    LabelledKnob   knobDepth { "DEPTH", juce::Colour(0xFFFFDD44) };
};

// ============================================================
//  LFOPanel
// ============================================================
class LFOPanel : public juce::Component
{
public:
    int lfoIndex;
    juce::ComboBox  waveCombo;
    LabelledKnob    knobSpeed { "SPEED", juce::Colour(0xFF88FFAA) };
    DepthSlot       slots[LFO_DEPTH_SLOTS];

    explicit LFOPanel(int idx);
    void paint(juce::Graphics&) override;
    void resized() override;
    void refreshTargetNames(const int blockTypes[NUM_FX_BLOCKS]);
};

// ============================================================
//  EnvPanel — envelope follower modulator
// ============================================================
class EnvPanel : public juce::Component
{
public:
    LabelledKnob    knobSens  { "SENS",  juce::Colour(0xFFFF8866) };
    LabelledKnob    knobSpeed { "SPEED", juce::Colour(0xFFFF8866) };
    DepthSlot       slots[ENV_DEPTH_SLOTS];

    EnvPanel();
    void paint(juce::Graphics&) override;
    void resized() override;
    void refreshTargetNames(const int blockTypes[NUM_FX_BLOCKS]);
};

// ============================================================
//  Editor
// ============================================================
class DragonflyFXAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit DragonflyFXAudioProcessorEditor(DragonflyFXAudioProcessor&);
    ~DragonflyFXAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    DragonflyFXAudioProcessor& processor;
    DragonflyLookAndFeel        laf;

    FXBlockPanel fxPanels[NUM_FX_BLOCKS] = {
        FXBlockPanel(0), FXBlockPanel(1), FXBlockPanel(2), FXBlockPanel(3)
    };
    LFOPanel lfoPanels[NUM_LFOS] = { LFOPanel(0), LFOPanel(1) };
    EnvPanel envPanel;

    // Global wet/dry mix (MASTER column)
    LabelledKnob knobMix { "MIX", juce::Colour(0xFF66DDEE) };

    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<CAtt> attFxType   [NUM_FX_BLOCKS];
    std::unique_ptr<BAtt> attFxBypass [NUM_FX_BLOCKS];
    std::unique_ptr<SAtt> attFxParam0 [NUM_FX_BLOCKS];
    std::unique_ptr<SAtt> attFxParam1 [NUM_FX_BLOCKS];

    std::unique_ptr<SAtt> attMix;

    std::unique_ptr<CAtt> attLfoWave [NUM_LFOS];
    std::unique_ptr<SAtt> attLfoSpeed[NUM_LFOS];
    std::unique_ptr<CAtt> attLfoTarget[NUM_LFOS][LFO_DEPTH_SLOTS];
    std::unique_ptr<SAtt> attLfoDepth [NUM_LFOS][LFO_DEPTH_SLOTS];

    std::unique_ptr<SAtt> attEnvSens, attEnvSpeed;
    std::unique_ptr<CAtt> attEnvTarget[ENV_DEPTH_SLOTS];
    std::unique_ptr<SAtt> attEnvDepth [ENV_DEPTH_SLOTS];

    int lastBlockTypes[NUM_FX_BLOCKS] = { -1, -1, -1, -1 };
    int lastBypass    [NUM_FX_BLOCKS] = { -1, -1, -1, -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragonflyFXAudioProcessorEditor)
};
