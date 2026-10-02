#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ============================================================
//  CricketLookAndFeel
// ============================================================
class CricketLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CricketLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAng, float endAng,
                          juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool highlighted, bool down) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& bg,
                              bool highlighted, bool down) override;
    void drawComboBox(juce::Graphics&, int w, int h,
                      bool down, int bx, int by, int bw, int bh,
                      juce::ComboBox&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
    juce::Font getLabelFont(juce::Label&) override;
};

// ============================================================
//  LabelledKnob — knob + label, with optional double-click reset
// ============================================================
class LabelledKnob : public juce::Component
{
public:
    juce::Slider  slider;
    juce::Label   label;
    juce::Colour  ringColour;
    float         resetValue = 0.0f; // used when doubleClickResetsToDefault = true

    LabelledKnob(const juce::String& name, juce::Colour col);
    void resized() override;
    void enableDoubleClickReset(float val);
    void enableSemitonePopup();  // show semitone value on hover/drag
};

// ============================================================
//  WaveSelector — labelled combo box
// ============================================================
class WaveSelector : public juce::Component
{
public:
    juce::ComboBox combo;
    juce::Label    label;
    WaveSelector(const juce::String& labelText,
                 juce::StringArray items = {"Sawtooth","Triangle","Noise","Square"});
    void resized() override;
};

// ============================================================
//  LEDChase
// ============================================================
class LEDChase : public juce::Component, public juce::Timer
{
public:
    LEDChase();
    ~LEDChase() override { stopTimer(); }
    void paint(juce::Graphics&) override;
    void timerCallback() override;
    void setSpeed(float hz);
    void setReverse(bool rev);  // true = Low range (reverse direction)

private:
    int   currentLED   = 0;
    float accumulator  = 0.0f;
    float speedHz      = 1.0f;
    bool  reversed     = false;
    static const int NUM_LEDS = 6;
    juce::Colour ledColours[NUM_LEDS] = {
        juce::Colour(0xFF, 0xDD, 0x00),
        juce::Colour(0x00, 0xAA, 0xFF),
        juce::Colour(0xFF, 0x22, 0x22),
        juce::Colour(0x22, 0xFF, 0x44),
        juce::Colour(0xFF, 0xDD, 0x00),
        juce::Colour(0xFF, 0x22, 0x22),
    };
};

// ============================================================
//  CricketAudioProcessorEditor
// ============================================================
class CricketAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    CricketAudioProcessorEditor(CricketAudioProcessor&);
    ~CricketAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    CricketAudioProcessor& processor;
    CricketLookAndFeel     laf;

    // ---- OSC 1 panel ----
    WaveSelector  waveOsc1      { "WAVE" };
    LabelledKnob  knobOsc1Pitch { "PITCH",    juce::Colour(0xFF, 0xFF, 0xFF) };
    LabelledKnob  knobFMDepth   { "FM DEPTH", juce::Colour(0xFF, 0xDD, 0x00) };
    LabelledKnob  knobOsc1Bend  { "BEND",     juce::Colour(0xAA, 0xCC, 0xFF) };

    // ---- OSC 2 panel ----
    WaveSelector  waveOsc2      { "WAVE" };
    LabelledKnob  knobOsc2Pitch { "PITCH",    juce::Colour(0xFF, 0xFF, 0xFF) };
    LabelledKnob  knobMix       { "OSC MIX",  juce::Colour(0xCC, 0xCC, 0xCC) };
    LabelledKnob  knobOsc2Bend  { "BEND",     juce::Colour(0xAA, 0xCC, 0xFF) };
    juce::ToggleButton btnOsc2Range { "OSC2 RANGE|High|Low" };

    // ---- ENVELOPE panel — amp env (top) + mod env (bottom) ----
    LabelledKnob  knobAttack   { "ATTACK",  juce::Colour(0xFF00CCFF) };
    LabelledKnob  knobDecay    { "DECAY",   juce::Colour(0xFF00CCFF) };
    LabelledKnob  knobSustain  { "SUSTAIN", juce::Colour(0xFF00CCFF) };
    LabelledKnob  knobRelease  { "RELEASE", juce::Colour(0xFF00CCFF) };
    // Mod envelope
    LabelledKnob  knobModA     { "ATTACK",  juce::Colour(0xFFFF8844) };
    LabelledKnob  knobModD     { "DECAY",   juce::Colour(0xFFFF8844) };
    LabelledKnob  knobModS     { "SUSTAIN", juce::Colour(0xFFFF8844) };
    LabelledKnob  knobModR     { "RELEASE", juce::Colour(0xFFFF8844) };
    juce::ToggleButton btnModLoop { "LOOP|Off|On" };
    // Mod amounts
    LabelledKnob  knobModOsc1  { "OSC1",    juce::Colour(0xFFFFAA44) };
    LabelledKnob  knobModOsc2  { "OSC2",    juce::Colour(0xFFFFAA44) };
    LabelledKnob  knobModFM    { "FM",      juce::Colour(0xFFFFAA44) };
    LabelledKnob  knobModLFO   { "LFO SPD", juce::Colour(0xFFFFAA44) };
    LabelledKnob  knobModWrap  { "WRAP",    juce::Colour(0xFFFFAA44) };

    // ---- LFO panel ----
    WaveSelector  waveLFO      { "WAVE" };
    LabelledKnob  knobLFOSpeed { "SPEED",   juce::Colour(0xFF88FF88) };
    LabelledKnob  knobLFODepth { "PITCH",   juce::Colour(0xFF88FF88) };
    LabelledKnob  knobLFOCutoff{ "CUTOFF",  juce::Colour(0xFF88FF88) };
    LabelledKnob  knobLFORes   { "RES",     juce::Colour(0xFF88FF88) };

    // ---- FILTER panel ----
    WaveSelector  filterTypeCombo { "TYPE",
        juce::StringArray{"Bypass","MS20 LP","MS20 HP","Comb"} };
    LabelledKnob  knobCutoff   { "CUTOFF",  juce::Colour(0xFFFF66AA) };
    LabelledKnob  knobFilterRes{ "RESONANCE",juce::Colour(0xFFFF66AA) };

    // Mod amount — cutoff (added to existing amount rows)
    LabelledKnob  knobModCutoff{ "CUTOFF",  juce::Colour(0xFFFFAA44) };

    // ---- OUTPUT panel ----
    LabelledKnob       knobMasterVol { "VOLUME",   juce::Colour(0xFF, 0xFF, 0xFF) };
    LabelledKnob       knobWrap      { "WRAP",     juce::Colour(0xCC, 0xCC, 0xCC) };
    LabelledKnob       knobVelSens   { "VELOCITY", juce::Colour(0xFF, 0xAA, 0x44) };
    juce::ToggleButton btnLogicOn    { "LOGIC|Off|On" };
    WaveSelector       switchLogicMode { "AND/OR/XOR",
                                         juce::StringArray{"AND","OR","XOR"} };

    // LED chase
    LEDChase ledChase;

    // Preset name display (updated when a .ckt file is loaded)
    juce::Label labelPreset;

    // APVTS attachments
    using SAtt  = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BAtt  = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SAtt> attOsc1Pitch, attOsc2Pitch, attFMDepth, attMix;
    std::unique_ptr<SAtt> attOsc1Bend, attOsc2Bend;
    std::unique_ptr<SAtt> attAttack, attDecay, attSustain, attRelease;
    std::unique_ptr<SAtt> attModA, attModD, attModS, attModR;
    std::unique_ptr<SAtt> attModOsc1, attModOsc2, attModFM, attModLFO, attModWrap, attModCutoff;
    std::unique_ptr<SAtt> attLFOSpeed, attLFODepth, attLFOCutoff, attLFORes;
    std::unique_ptr<SAtt> attMasterVol, attWrap, attVelSens;
    std::unique_ptr<SAtt> attCutoff, attFilterRes;
    std::unique_ptr<BAtt> attLogicOn, attOsc2Range, attModLoop;
    std::unique_ptr<CAtt> attWaveOsc1, attWaveOsc2, attWaveLFO, attLogicMode, attFilterType;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CricketAudioProcessorEditor)
};
