#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ============================================================
//  Moth — '80s sci-fi visual language
//  Reference points: the green-phosphor Nostromo terminals of
//  ALIEN (MU-TH-UR), and the bronze/amber baroque-industrial
//  palette of Lynch's DUNE (1984). CRT scanlines, engraved
//  metal panels, machined dials, terminal monospace type.
// ============================================================

namespace MothColours {
    const juce::Colour panelBlack   { 0xFF0a0c0a };  // deep CRT black-green
    const juce::Colour panelDark    { 0xFF12150f };
    const juce::Colour metalBronze  { 0xFF6e5a2e };  // Dune bronze
    const juce::Colour metalBronzeHi{ 0xFFb89a52 };
    const juce::Colour metalEdge    { 0xFF3a3320 };
    const juce::Colour phosphor     { 0xFF7dff8e };  // Alien phosphor green
    const juce::Colour phosphorDim  { 0xFF2f6e38 };
    const juce::Colour amber        { 0xFFffb54a };  // amber accent
    const juce::Colour amberDim     { 0xFF7a5520 };
    const juce::Colour textGreen     { 0xFFa6f5b0 };
    const juce::Colour textBronze    { 0xFFd8c089 };
    const juce::Colour warningRed    { 0xFFff5a44 };
}

// ============================================================
//  MothLookAndFeel — machined dials, CRT toggles, terminal combos
// ============================================================
class MothLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MothLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAng, float endAng,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool highlighted, bool down) override;

    void drawComboBox(juce::Graphics&, int w, int h, bool down,
                      int bx, int by, int bw, int bh, juce::ComboBox&) override;

    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getLabelFont(juce::Label&) override;
    void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;

    juce::Colour ringColour = MothColours::amber;
};

// ============================================================
//  TerminalKnob — machined dial with engraved label plate
// ============================================================
class TerminalKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::String caption;
    juce::Colour accent;

    TerminalKnob(const juce::String& name, juce::Colour col);
    void resized() override;
    void paint(juce::Graphics&) override;
};

// ============================================================
//  TerminalCombo — labelled terminal-style selector
// ============================================================
class TerminalCombo : public juce::Component
{
public:
    juce::ComboBox combo;
    juce::String   caption;

    TerminalCombo(const juce::String& name, juce::StringArray items);
    void resized() override;
    void paint(juce::Graphics&) override;
};

// ============================================================
//  PowerToggle — a CRT-style section enable switch with LED
// ============================================================
class PowerToggle : public juce::ToggleButton
{
public:
    PowerToggle() = default;
};

// ============================================================
//  CRTMeter — phosphor output meter with scanline
// ============================================================
class CRTMeter : public juce::Component, private juce::Timer
{
public:
    CRTMeter(MothAudioProcessor& p);
    ~CRTMeter() override { stopTimer(); }
    void paint(juce::Graphics&) override;
    void timerCallback() override;
private:
    MothAudioProcessor& processor;
    float dispL = 0.0f, dispR = 0.0f;
};

// ============================================================
//  Editor
// ============================================================
class MothAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
    MothAudioProcessorEditor(MothAudioProcessor&);
    ~MothAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void drawPanel(juce::Graphics&, juce::Rectangle<int> r,
                   const juce::String& title, bool enabled, float scanPhase);

    MothAudioProcessor& processor;
    MothLookAndFeel laf;

    // Distortion section
    PowerToggle   distPower;
    TerminalKnob  knobDistortion { "DISTORTION", MothColours::amber };
    TerminalKnob  knobFilter     { "FILTER",     MothColours::amber };
    TerminalKnob  knobDistVol    { "VOLUME",     MothColours::amber };

    // Phaser section
    PowerToggle   phaserPower;
    TerminalKnob  knobSpeed     { "SPEED",      MothColours::phosphor };
    TerminalKnob  knobDepth     { "DEPTH",      MothColours::phosphor };
    TerminalKnob  knobCharacter { "CHARACTER",  MothColours::phosphor };
    TerminalKnob  knobFeedback  { "FEEDBACK",   MothColours::phosphor };
    TerminalCombo comboShape    { "LFO WAVE",
        juce::StringArray{"SINE","TRIANGLE","SQUARE","SAWTOOTH","RANDOM"} };

    // Output
    TerminalKnob  knobMix { "MIX",    MothColours::textBronze };
    TerminalKnob  knobOut { "OUTPUT", MothColours::textBronze };
    CRTMeter      meter;

    float scanPhase = 0.0f;

    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SAtt> attDistortion, attFilter, attDistVol;
    std::unique_ptr<SAtt> attSpeed, attDepth, attCharacter, attFeedback;
    std::unique_ptr<SAtt> attMix, attOut;
    std::unique_ptr<BAtt> attDistOn, attPhaserOn;
    std::unique_ptr<CAtt> attShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MothAudioProcessorEditor)
};
