#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ============================================================
//  SilkwormLookAndFeel — dark / indigo / teal theme
// ============================================================
class SilkwormLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SilkwormLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAng, float endAng,
                          juce::Slider&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
    juce::Font getLabelFont(juce::Label&) override;
};

// ============================================================
//  LabelledKnob — rotary knob + label underneath
// ============================================================
class SWKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;
    juce::Colour ringColour;

    SWKnob(const juce::String& name, juce::Colour col);
    void resized() override;
};

// ============================================================
//  SilkwormAudioProcessorEditor
// ============================================================
class SilkwormAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SilkwormAudioProcessorEditor(SilkwormAudioProcessor&);
    ~SilkwormAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SilkwormAudioProcessor& processor;
    SilkwormLookAndFeel     laf;

    // ---- Reverb panel ----
    SWKnob knobPredelay  { "PRE-DLY",   juce::Colour(0xFF6688CC) };
    SWKnob knobDecay     { "DECAY",     juce::Colour(0xFF6688CC) };
    SWKnob knobDensity   { "DENSITY",   juce::Colour(0xFF6688CC) };

    // ---- Character panel ----
    SWKnob knobCharacter { "CHARACTER", juce::Colour(0xFFAA77CC) };
    SWKnob knobClock     { "CLOCK",     juce::Colour(0xFFAA77CC) };
    SWKnob knobCutoff    { "CUTOFF",    juce::Colour(0xFFAA77CC) };
    SWKnob knobMod       { "MOD",       juce::Colour(0xFFAA77CC) };

    // ---- Output panel ----
    SWKnob knobMix       { "MIX",       juce::Colour(0xFF55BBAA) };
    SWKnob knobVolume    { "VOLUME",    juce::Colour(0xFF55BBAA) };

    // APVTS attachments
    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SAtt> attPredelay, attDecay, attDensity;
    std::unique_ptr<SAtt> attCharacter, attClock, attCutoff, attMod;
    std::unique_ptr<SAtt> attMix, attVolume;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilkwormAudioProcessorEditor)
};
