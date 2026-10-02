#include "PluginEditor.h"

// ============================================================
//  SilkwormLookAndFeel
// ============================================================
SilkwormLookAndFeel::SilkwormLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF151520));
    setColour(juce::Label::textColourId,                 juce::Colour(0xFFBBBBCC));
}

void SilkwormLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAng, float endAng, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float)x, (float)y,
                                          (float)w, (float)h).reduced(4.0f);
    float radius  = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();

    // Background circle
    g.setColour(juce::Colour(0xFF222233));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

    // Arc track
    float arcR = radius * 0.85f;
    juce::Path arcBg;
    arcBg.addArc(centreX - arcR, centreY - arcR, arcR * 2, arcR * 2,
                 startAng, endAng, true);
    g.setColour(juce::Colour(0xFF333344));
    g.strokePath(arcBg, juce::PathStrokeType(3.0f));

    // Bipolar detection
    bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);

    if (isBipolar) {
        float centrePos = (float)(-slider.getMinimum()
                        / (slider.getMaximum() - slider.getMinimum()));
        float centreAng = startAng + (endAng - startAng) * centrePos;
        float valueAng  = startAng + (endAng - startAng) * sliderPos;

        if (std::abs(sliderPos - centrePos) > 0.001f) {
            juce::Path arcFill;
            float a1 = std::min(centreAng, valueAng);
            float a2 = std::max(centreAng, valueAng);
            arcFill.addArc(centreX - arcR, centreY - arcR, arcR * 2, arcR * 2,
                           a1, a2, true);
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(arcFill, juce::PathStrokeType(3.0f));
        }
        // Centre tick
        float cx = centreX + std::sin(centreAng - juce::MathConstants<float>::halfPi) * arcR;
        float cy = centreY - std::cos(centreAng - juce::MathConstants<float>::halfPi) * arcR;
        g.setColour(juce::Colour(0xFF555566));
        g.fillEllipse(cx - 2, cy - 2, 4, 4);
    } else {
        // Standard arc from start to current value
        juce::Path arcFill;
        arcFill.addArc(centreX - arcR, centreY - arcR, arcR * 2, arcR * 2,
                       startAng, startAng + (endAng - startAng) * sliderPos, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(arcFill, juce::PathStrokeType(3.0f));
    }

    // Knob body
    float knobR = radius * 0.60f;
    g.setColour(juce::Colour(0xFF333344));
    g.fillEllipse(centreX - knobR, centreY - knobR, knobR * 2, knobR * 2);
    g.setColour(juce::Colour(0xFF555566));
    g.drawEllipse(centreX - knobR, centreY - knobR, knobR * 2, knobR * 2, 1.5f);

    // Pointer line
    float angle = startAng + (endAng - startAng) * sliderPos
                - juce::MathConstants<float>::halfPi;
    float px = centreX + std::cos(angle) * knobR * 0.7f;
    float py = centreY + std::sin(angle) * knobR * 0.7f;
    g.setColour(juce::Colour(0xFFDDDDEE));
    g.drawLine(centreX, centreY, px, py, 2.5f);
    g.fillEllipse(centreX - 2, centreY - 2, 4, 4);
}

void SilkwormLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& lbl)
{
    g.setColour(lbl.findColour(juce::Label::textColourId));
    g.setFont(getLabelFont(lbl));
    g.drawFittedText(lbl.getText(), lbl.getLocalBounds(),
                     lbl.getJustificationType(), 2);
}

juce::Font SilkwormLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::Font::getDefaultMonospacedFontName(),
                      9.5f, juce::Font::bold);
}

// ============================================================
//  SWKnob
// ============================================================
SWKnob::SWKnob(const juce::String& name, juce::Colour col)
    : ringColour(col)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                             9.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xFF999AAA));
    addAndMakeVisible(label);
}

void SWKnob::resized()
{
    auto b = getLocalBounds();
    label.setBounds(b.removeFromBottom(14));
    slider.setBounds(b);
}

// ============================================================
//  Panel drawing helper
// ============================================================
static void drawPanel(juce::Graphics& g, juce::Rectangle<int> r,
                      const juce::String& title, juce::Colour accent)
{
    g.setColour(accent.withAlpha(0.12f));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(accent.withAlpha(0.40f));
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 6.0f, 1.0f);

    auto titleBar = r.removeFromTop(18).toFloat();
    g.setColour(accent.withAlpha(0.25f));
    g.fillRoundedRectangle(titleBar, 5.0f);
    g.setColour(juce::Colour(0xFFCCCCDD));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                         10.0f, juce::Font::bold));
    g.drawText(title, titleBar.toNearestInt().reduced(6, 0),
               juce::Justification::centredLeft, true);
}

// ============================================================
//  Editor constructor
// ============================================================
SilkwormAudioProcessorEditor::SilkwormAudioProcessorEditor(
    SilkwormAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&laf);
    setSize(580, 290);

    addAndMakeVisible(knobPredelay);
    addAndMakeVisible(knobDecay);
    addAndMakeVisible(knobDensity);
    addAndMakeVisible(knobCharacter);
    addAndMakeVisible(knobClock);
    addAndMakeVisible(knobCutoff);
    addAndMakeVisible(knobMod);
    addAndMakeVisible(knobMix);
    addAndMakeVisible(knobVolume);

    // Double-click reset for bipolar cutoff knob
    knobCutoff.slider.setDoubleClickReturnValue(true, 0.0);

    attPredelay  = std::make_unique<SAtt>(p.apvts, ParamID::predelay,   knobPredelay.slider);
    attDecay     = std::make_unique<SAtt>(p.apvts, ParamID::decay,      knobDecay.slider);
    attDensity   = std::make_unique<SAtt>(p.apvts, ParamID::density,    knobDensity.slider);
    attCharacter = std::make_unique<SAtt>(p.apvts, ParamID::character_, knobCharacter.slider);
    attClock     = std::make_unique<SAtt>(p.apvts, ParamID::clockSpeed, knobClock.slider);
    attCutoff    = std::make_unique<SAtt>(p.apvts, ParamID::cutoffOfs,  knobCutoff.slider);
    attMod       = std::make_unique<SAtt>(p.apvts, ParamID::modAmount,  knobMod.slider);
    attMix       = std::make_unique<SAtt>(p.apvts, ParamID::mix,        knobMix.slider);
    attVolume    = std::make_unique<SAtt>(p.apvts, ParamID::volume,     knobVolume.slider);
}

SilkwormAudioProcessorEditor::~SilkwormAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

// ============================================================
//  paint
// ============================================================
void SilkwormAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF151520));

    // Title bar
    g.setColour(juce::Colour(0xFF1c1c2a));
    g.fillRect(0, 0, getWidth(), 30);
    g.setColour(juce::Colour(0xFF2a2a3a));
    g.drawLine(0, 30, (float)getWidth(), 30, 1.0f);

    // Title text
    g.setColour(juce::Colour(0xFF8899DD));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                         18.0f, juce::Font::bold));
    g.drawText("SILKWORM", 12, 4, 140, 22,
               juce::Justification::centredLeft, false);

    // Version
    g.setColour(juce::Colour(0xFF667788));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                         10.0f, juce::Font::plain));
    g.drawText(juce::String("v") + JucePlugin_VersionString,
               140, 10, 50, 12, juce::Justification::centredLeft, false);

    // Subtitle
    g.setColour(juce::Colour(0xFF556677));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                         9.0f, juce::Font::plain));
    g.drawText("lo-fi allpass reverb", 200, 10, 140, 12,
               juce::Justification::centredLeft, false);
    g.drawText("MURMUR ENGINEERING", getWidth() - 192, 10, 180, 12,
               juce::Justification::centredRight, false);

    // Panel backgrounds
    const int top = 34, h = 248;
    drawPanel(g, {  4, top, 210, h}, "REVERB",    juce::Colour(0xFF334477));
    drawPanel(g, {220, top, 148, h}, "CHARACTER", juce::Colour(0xFF553366));
    drawPanel(g, {374, top, 202, h}, "OUTPUT",    juce::Colour(0xFF336655));
}

// ============================================================
//  resized
// ============================================================
void SilkwormAudioProcessorEditor::resized()
{
    const int top  = 34;
    const int kW   = 62;
    const int kH   = 76;
    const int panY = top + 24;  // below panel title bar

    // ---- REVERB panel (x=4, w=210) ----
    {
        int x = 10;
        knobPredelay.setBounds(x,          panY, kW, kH);
        knobDecay.setBounds   (x + kW + 4, panY, kW, kH);
        knobDensity.setBounds (x + 34,     panY + kH + 8, kW, kH);
    }

    // ---- CHARACTER panel (x=220, w=148) ----
    {
        int x = 226;
        knobCharacter.setBounds(x,          panY,          kW, kH);
        knobClock.setBounds    (x + kW + 4, panY,          kW, kH);
        knobCutoff.setBounds   (x,          panY + kH + 8, kW, kH);
        knobMod.setBounds      (x + kW + 4, panY + kH + 8, kW, kH);
    }

    // ---- OUTPUT panel (x=374, w=202) ----
    {
        int x = 380;
        knobMix.setBounds   (x,          panY, kW, kH);
        knobVolume.setBounds(x + kW + 4, panY, kW, kH);
    }
}
