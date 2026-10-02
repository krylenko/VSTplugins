#include "PluginEditor.h"

// ============================================================
//  CricketLookAndFeel
// ============================================================
CricketLookAndFeel::CricketLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId,  juce::Colour(0xFF1a1a1a));
    setColour(juce::Slider::rotarySliderFillColourId,     juce::Colour(0xFF444444));
    setColour(juce::Slider::rotarySliderOutlineColourId,  juce::Colour(0xFF222222));
    setColour(juce::Slider::thumbColourId,                juce::Colour(0xFFFFDD00));
    setColour(juce::Label::textColourId,                  juce::Colour(0xFFCCCCCC));
    setColour(juce::ComboBox::backgroundColourId,         juce::Colour(0xFF2a2a2a));
    setColour(juce::ComboBox::textColourId,               juce::Colour(0xFFCCCCCC));
    setColour(juce::ComboBox::outlineColourId,            juce::Colour(0xFF555555));
    setColour(juce::PopupMenu::backgroundColourId,        juce::Colour(0xFF2a2a2a));
    setColour(juce::PopupMenu::textColourId,              juce::Colour(0xFFCCCCCC));
    setColour(juce::ToggleButton::textColourId,           juce::Colour(0xFFCCCCCC));
    setColour(juce::ToggleButton::tickColourId,           juce::Colour(0xFFFFDD00));
    setColour(juce::ToggleButton::tickDisabledColourId,   juce::Colour(0xFF555555));
    setColour(juce::TextButton::buttonColourId,           juce::Colour(0xFF2a2a2a));
    setColour(juce::TextButton::buttonOnColourId,         juce::Colour(0xFFFFDD00));
    setColour(juce::TextButton::textColourOffId,          juce::Colour(0xFFCCCCCC));
    setColour(juce::TextButton::textColourOnId,           juce::Colour(0xFF111111));
}

void CricketLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAng, float endAng, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(4.0f);
    float radius  = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();

    // Background circle
    g.setColour(juce::Colour(0xFF333333));
    g.fillEllipse(centreX-radius, centreY-radius, radius*2, radius*2);

    float arcR = radius * 0.85f;
    bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);

    if (isBipolar) {
        // Arc from centre position outward in the direction of the value
        float centrePos = (float)(-slider.getMinimum() /
                          (slider.getMaximum() - slider.getMinimum()));
        float centreAng = startAng + (endAng - startAng) * centrePos;
        float valueAng  = startAng + (endAng - startAng) * sliderPos;

        if (std::abs(sliderPos - centrePos) > 0.001f) {
            juce::Path arcPath;
            float a1 = std::min(centreAng, valueAng);
            float a2 = std::max(centreAng, valueAng);
            arcPath.addArc(centreX-arcR, centreY-arcR, arcR*2, arcR*2, a1, a2, true);
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(arcPath, juce::PathStrokeType(3.0f));
        }
        // Centre tick mark
        float cx = centreX + std::sin(centreAng - juce::MathConstants<float>::halfPi) * arcR;
        float cy = centreY - std::cos(centreAng - juce::MathConstants<float>::halfPi) * arcR;
        g.setColour(juce::Colour(0xFF555555));
        g.fillEllipse(cx-2, cy-2, 4, 4);
    } else {
        // Standard arc from start to current value
        juce::Path arcPath;
        arcPath.addArc(centreX-arcR, centreY-arcR, arcR*2, arcR*2,
                       startAng, startAng + (endAng-startAng)*sliderPos, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(arcPath, juce::PathStrokeType(3.0f));
    }

    // Knob body
    float knobR = radius * 0.65f;
    g.setColour(juce::Colour(0xFF444444));
    g.fillEllipse(centreX-knobR, centreY-knobR, knobR*2, knobR*2);
    g.setColour(juce::Colour(0xFF666666));
    g.drawEllipse(centreX-knobR, centreY-knobR, knobR*2, knobR*2, 1.5f);

    float angle = startAng + (endAng-startAng)*sliderPos - juce::MathConstants<float>::halfPi;
    float px = centreX + std::cos(angle)*knobR*0.7f;
    float py = centreY + std::sin(angle)*knobR*0.7f;
    g.setColour(juce::Colour(0xFFEEEEEE));
    g.drawLine(centreX, centreY, px, py, 2.5f);
    g.fillEllipse(centreX-2, centreY-2, 4, 4);
}

void CricketLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& btn, bool highlighted, bool)
{
    const int w = btn.getWidth();
    bool on = btn.getToggleState();

    // Parse button text: "NAME|TopLabel|BottomLabel" or just "NAME"
    juce::String btnText = btn.getButtonText();
    juce::String name = btnText, topLabel = "Off", botLabel = "On";
    int pipe = btnText.indexOf("|");
    if (pipe >= 0) {
        juce::StringArray parts;
        parts.addTokens(btnText, "|", "");
        if (parts.size() >= 3) { name = parts[0]; topLabel = parts[1]; botLabel = parts[2]; }
    }

    // Name label at top
    g.setColour(juce::Colour(0xFFAAAAAA));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText(name, 0, 0, w, 14, juce::Justification::centred, true);

    // Switch body centred below name
    const float sw = 20.0f, sh = 34.0f;
    const float sx = w * 0.5f - sw * 0.5f, sy = 16.0f;

    g.setColour(juce::Colour(0xFF2a2a2a));
    g.fillRoundedRectangle(sx, sy, sw, sh, 4.0f);
    g.setColour(juce::Colour(0xFF555555));
    g.drawRoundedRectangle(sx, sy, sw, sh, 4.0f, 1.5f);

    // Paddle: up = off (topLabel), down = on (botLabel)
    float paddleH = sh * 0.44f;
    float py = on ? sy + sh - paddleH - 2.0f : sy + 2.0f;
    g.setColour(highlighted ? juce::Colour(0xFFFFEE44)
                : (on ? juce::Colour(0xFFFFDD00) : juce::Colour(0xFF888888)));
    g.fillRoundedRectangle(sx + 3.0f, py, sw - 6.0f, paddleH, 3.0f);

    // Position labels to the right of switch body
    float labelX = sx + sw + 4.0f;
    int labelW = std::max(1, (int)(w - labelX - 2.0f));
    g.setColour(juce::Colour(0xFF888888));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
    g.drawText(topLabel, (int)labelX, (int)sy,            labelW, 14, juce::Justification::centredLeft, true);
    g.drawText(botLabel, (int)labelX, (int)(sy + sh - 14),labelW, 14, juce::Justification::centredLeft, true);
}

void CricketLookAndFeel::drawButtonBackground(juce::Graphics& g,
    juce::Button& btn, const juce::Colour&, bool highlighted, bool down)
{
    auto b = btn.getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(down ? juce::Colour(0xFFFFEE00)
                     : (highlighted ? juce::Colour(0xFFFFCC00) : juce::Colour(0xFFBB9900)));
    g.fillRoundedRectangle(b, 6.0f);
    g.setColour(juce::Colour(0x33000000));
    g.drawRoundedRectangle(b, 6.0f, 1.5f);
}

void CricketLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h,
    bool, int, int, int, int, juce::ComboBox& box)
{
    auto b = juce::Rectangle<float>(0,0,(float)w,(float)h);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 1.0f);
    float ax=w-16, ay=h*0.5f;
    juce::Path arrow;
    arrow.addTriangle(ax,ay-3, ax+6,ay-3, ax+3,ay+3);
    g.setColour(box.findColour(juce::ComboBox::textColourId));
    g.fillPath(arrow);
}

void CricketLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& lbl)
{
    g.setColour(lbl.findColour(juce::Label::textColourId));
    g.setFont(getLabelFont(lbl));
    g.drawFittedText(lbl.getText(), lbl.getLocalBounds(),
                     lbl.getJustificationType(), 2);
}

juce::Font CricketLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold);
}

// ============================================================
//  LabelledKnob
// ============================================================
LabelledKnob::LabelledKnob(const juce::String& name, juce::Colour col)
    : ringColour(col)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
    addAndMakeVisible(label);
}

void LabelledKnob::enableDoubleClickReset(float val)
{
    resetValue = val;
    slider.setDoubleClickReturnValue(true, val);
}

void LabelledKnob::enableSemitonePopup()
{
    // Show a popup bubble while hovering or dragging, formatted as semitones
    slider.setPopupDisplayEnabled(true, true, nullptr);
    slider.textFromValueFunction = [](double val) -> juce::String {
        int semi = (int)std::round(val);
        if (semi == 0)  return juce::String("0 st");
        if (semi > 0)   return juce::String("+") + juce::String(semi) + " st";
        return juce::String(semi) + " st";
    };
    slider.valueFromTextFunction = [](const juce::String& text) -> double {
        return text.getDoubleValue();
    };
}

void LabelledKnob::resized()
{
    auto b = getLocalBounds();
    label.setBounds(b.removeFromBottom(16));
    slider.setBounds(b);
}

// ============================================================
//  WaveSelector
// ============================================================
WaveSelector::WaveSelector(const juce::String& labelText, juce::StringArray items)
{
    combo.addItemList(items, 1);
    combo.setSelectedId(items.size(), juce::dontSendNotification);
    addAndMakeVisible(combo);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
    addAndMakeVisible(label);
}

void WaveSelector::resized()
{
    auto b = getLocalBounds();
    label.setBounds(b.removeFromTop(16));
    combo.setBounds(b);
}

// ============================================================
//  LEDChase
// ============================================================
LEDChase::LEDChase() { startTimerHz(30); }

void LEDChase::setSpeed(float hz) { speedHz = hz; }

void LEDChase::setReverse(bool rev) { reversed = rev; }

void LEDChase::timerCallback()
{
    accumulator += speedHz / 30.0f;
    while (accumulator >= 1.0f) {
        if (reversed)
            currentLED = (currentLED + NUM_LEDS - 1) % NUM_LEDS;
        else
            currentLED = (currentLED + 1) % NUM_LEDS;
        accumulator -= 1.0f;
    }
    repaint();
}

void LEDChase::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    float spacing = b.getWidth() / (float)NUM_LEDS;
    float ledSize = 12.0f;

    for (int i = 0; i < NUM_LEDS; ++i) {
        float cx = b.getX() + spacing*i + spacing*0.5f;
        float cy = b.getCentreY();
        bool lit = (i == currentLED);
        g.setColour(juce::Colour(0x88000000));
        g.fillEllipse(cx-ledSize*0.5f+1, cy-ledSize*0.5f+1, ledSize, ledSize);
        g.setColour(lit ? ledColours[i] : ledColours[i].withAlpha(0.15f));
        g.fillEllipse(cx-ledSize*0.5f, cy-ledSize*0.5f, ledSize, ledSize);
        if (lit) {
            g.setColour(juce::Colours::white.withAlpha(0.4f));
            g.fillEllipse(cx-ledSize*0.28f, cy-ledSize*0.38f, ledSize*0.28f, ledSize*0.28f);
        }
        g.setColour(juce::Colour(0x99000000));
        g.drawEllipse(cx-ledSize*0.5f, cy-ledSize*0.5f, ledSize, ledSize, 1.0f);
    }
}

// ============================================================
//  Panel header drawing helper
// ============================================================
static void drawPanel(juce::Graphics& g, juce::Rectangle<int> r,
                      const juce::String& title, juce::Colour accent)
{
    g.setColour(accent.withAlpha(0.18f));
    g.fillRoundedRectangle(r.toFloat(), 5.0f);
    g.setColour(accent.withAlpha(0.55f));
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 5.0f, 1.0f);
    // Title bar
    auto titleBar = r.removeFromTop(18).toFloat();
    g.setColour(accent.withAlpha(0.35f));
    g.fillRoundedRectangle(titleBar, 4.0f);
    g.setColour(juce::Colour(0xFFDDDDDD));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText(title, titleBar.toNearestInt().reduced(4,0),
               juce::Justification::centredLeft, true);
}

// ============================================================
//  Editor constructor
// ============================================================
CricketAudioProcessorEditor::CricketAudioProcessorEditor(CricketAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&laf);
    setSize(982, 400);

    // Double-click resets pitch knobs to 0 (MIDI-accurate); hover shows semitone value
    knobOsc1Pitch.enableDoubleClickReset(0.0f);
    knobOsc1Pitch.enableSemitonePopup();
    knobOsc2Pitch.enableDoubleClickReset(0.0f);
    knobOsc2Pitch.enableSemitonePopup();

    // Add all children
    addAndMakeVisible(waveOsc1);
    addAndMakeVisible(knobOsc1Pitch);
    addAndMakeVisible(knobFMDepth);
    addAndMakeVisible(knobOsc1Bend);

    addAndMakeVisible(waveOsc2);
    addAndMakeVisible(knobOsc2Pitch);
    addAndMakeVisible(knobMix);
    addAndMakeVisible(knobOsc2Bend);
    addAndMakeVisible(btnOsc2Range);

    addAndMakeVisible(knobAttack);
    addAndMakeVisible(knobDecay);
    addAndMakeVisible(knobSustain);
    addAndMakeVisible(knobRelease);
    addAndMakeVisible(knobModA);
    addAndMakeVisible(knobModD);
    addAndMakeVisible(knobModS);
    addAndMakeVisible(knobModR);
    addAndMakeVisible(btnModLoop);
    addAndMakeVisible(knobModOsc1);
    addAndMakeVisible(knobModOsc2);
    addAndMakeVisible(knobModFM);
    addAndMakeVisible(knobModLFO);
    addAndMakeVisible(knobModWrap);

    addAndMakeVisible(waveLFO);
    addAndMakeVisible(knobLFOSpeed);
    addAndMakeVisible(knobLFODepth);
    addAndMakeVisible(knobLFOCutoff);
    addAndMakeVisible(knobLFORes);

    addAndMakeVisible(filterTypeCombo);
    addAndMakeVisible(knobCutoff);
    addAndMakeVisible(knobFilterRes);
    addAndMakeVisible(knobModCutoff);

    addAndMakeVisible(knobMasterVol);
    addAndMakeVisible(knobWrap);
    addAndMakeVisible(knobVelSens);
    addAndMakeVisible(btnLogicOn);
    // AND/OR/XOR three-way switch — no extra label (OUTPUT panel header is enough)
    switchLogicMode.label.setText("", juce::dontSendNotification);
    switchLogicMode.label.setVisible(false);
    addAndMakeVisible(switchLogicMode);

    addAndMakeVisible(ledChase);

    // Attachments
    attOsc1Pitch = std::make_unique<SAtt>(p.apvts, ParamID::osc1Pitch, knobOsc1Pitch.slider);
    attFMDepth   = std::make_unique<SAtt>(p.apvts, ParamID::fmDepth,   knobFMDepth.slider);
    attOsc1Bend  = std::make_unique<SAtt>(p.apvts, ParamID::osc1BendRange, knobOsc1Bend.slider);
    attOsc2Pitch = std::make_unique<SAtt>(p.apvts, ParamID::osc2Pitch, knobOsc2Pitch.slider);
    attMix       = std::make_unique<SAtt>(p.apvts, ParamID::oscMix,    knobMix.slider);
    attOsc2Bend  = std::make_unique<SAtt>(p.apvts, ParamID::osc2BendRange, knobOsc2Bend.slider);
    attAttack    = std::make_unique<SAtt>(p.apvts, ParamID::attack,    knobAttack.slider);
    attDecay     = std::make_unique<SAtt>(p.apvts, ParamID::decay,     knobDecay.slider);
    attSustain   = std::make_unique<SAtt>(p.apvts, ParamID::sustain,   knobSustain.slider);
    attRelease   = std::make_unique<SAtt>(p.apvts, ParamID::release_,  knobRelease.slider);
    attModA      = std::make_unique<SAtt>(p.apvts, ParamID::modAttack,      knobModA.slider);
    attModD      = std::make_unique<SAtt>(p.apvts, ParamID::modDecay,       knobModD.slider);
    attModS      = std::make_unique<SAtt>(p.apvts, ParamID::modSustain,     knobModS.slider);
    attModR      = std::make_unique<SAtt>(p.apvts, ParamID::modRelease_,    knobModR.slider);
    attModOsc1   = std::make_unique<SAtt>(p.apvts, ParamID::modToOsc1Pitch, knobModOsc1.slider);
    attModOsc2   = std::make_unique<SAtt>(p.apvts, ParamID::modToOsc2Pitch, knobModOsc2.slider);
    attModFM     = std::make_unique<SAtt>(p.apvts, ParamID::modToFMDepth,   knobModFM.slider);
    attModLFO    = std::make_unique<SAtt>(p.apvts, ParamID::modToLFOSpeed,  knobModLFO.slider);
    attModWrap   = std::make_unique<SAtt>(p.apvts, ParamID::modToWrap,      knobModWrap.slider);
    attModCutoff = std::make_unique<SAtt>(p.apvts, ParamID::modToCutoff,    knobModCutoff.slider);
    attLFOSpeed  = std::make_unique<SAtt>(p.apvts, ParamID::lfoSpeed,  knobLFOSpeed.slider);
    attLFODepth  = std::make_unique<SAtt>(p.apvts, ParamID::lfoDepth,  knobLFODepth.slider);
    attLFOCutoff = std::make_unique<SAtt>(p.apvts, ParamID::lfoToCutoff, knobLFOCutoff.slider);
    attLFORes    = std::make_unique<SAtt>(p.apvts, ParamID::lfoToRes,    knobLFORes.slider);
    attCutoff    = std::make_unique<SAtt>(p.apvts, ParamID::filterCutoff, knobCutoff.slider);
    attFilterRes = std::make_unique<SAtt>(p.apvts, ParamID::filterRes,    knobFilterRes.slider);
    attMasterVol = std::make_unique<SAtt>(p.apvts, ParamID::masterVol,      knobMasterVol.slider);
    attWrap      = std::make_unique<SAtt>(p.apvts, ParamID::wrap,            knobWrap.slider);
    attVelSens   = std::make_unique<SAtt>(p.apvts, ParamID::velSensitivity,  knobVelSens.slider);

    attLogicOn    = std::make_unique<BAtt>(p.apvts, ParamID::logicOn,   btnLogicOn);
    attOsc2Range  = std::make_unique<BAtt>(p.apvts, ParamID::osc2Range, btnOsc2Range);
    attModLoop    = std::make_unique<BAtt>(p.apvts, ParamID::modLoop,   btnModLoop);

    attWaveOsc1   = std::make_unique<CAtt>(p.apvts, ParamID::osc1Wave,  waveOsc1.combo);
    attWaveOsc2   = std::make_unique<CAtt>(p.apvts, ParamID::osc2Wave,  waveOsc2.combo);
    attWaveLFO    = std::make_unique<CAtt>(p.apvts, ParamID::lfoWave,   waveLFO.combo);
    attLogicMode  = std::make_unique<CAtt>(p.apvts, ParamID::logicMode, switchLogicMode.combo);
    attFilterType = std::make_unique<CAtt>(p.apvts, ParamID::filterType, filterTypeCombo.combo);

    // Sync LED speed to LFO speed param
    knobLFOSpeed.slider.onValueChange = [this] {
        ledChase.setSpeed((float)knobLFOSpeed.slider.getValue());
    };
    ledChase.setSpeed((float)*processor.apvts.getRawParameterValue(ParamID::lfoSpeed));

    // Preset name label — lives in the title bar, updated by timer
    labelPreset.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    labelPreset.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    labelPreset.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(labelPreset);

    startTimerHz(20);
}

CricketAudioProcessorEditor::~CricketAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void CricketAudioProcessorEditor::timerCallback()
{
    // Poll OSC2 Range switch and reverse LED direction when Low
    bool isLow = (*processor.apvts.getRawParameterValue(ParamID::osc2Range) > 0.5f);
    ledChase.setReverse(isLow);

    // Update preset name display
    labelPreset.setText(processor.presetName, juce::dontSendNotification);
}

// ============================================================
//  paint
// ============================================================
void CricketAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1a1a1a));

    // Title bar
    g.setColour(juce::Colour(0xFF252525));
    g.fillRect(0, 0, getWidth(), 32);
    g.setColour(juce::Colour(0xFF333333));
    g.drawLine(0, 32, getWidth(), 32, 1.0f);
    g.setColour(juce::Colour(0xFFFFDD00));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 18.0f, juce::Font::bold));
    g.drawText("CRICKET", 12, 5, 100, 22, juce::Justification::centredLeft, false);
    g.setColour(juce::Colour(0xFF888888));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    g.drawText(juce::String("v") + JucePlugin_VersionString,
               118, 12, 50, 12, juce::Justification::centredLeft, false);
    g.drawText("MURMUR ENGINEERING",
               getWidth() - 190, 12, 178, 12, juce::Justification::centredRight, false);

    // Panel backgrounds
    const int top = 36, h = 320;
    drawPanel(g, {4,   top, 138, h}, "OSC 1",    juce::Colour(0xFF4466AA));
    drawPanel(g, {146, top, 138, h}, "OSC 2",    juce::Colour(0xFF664422));
    drawPanel(g, {288, top, 264, h}, "ENVELOPE", juce::Colour(0xFF226644));
    drawPanel(g, {556, top, 138, h}, "LFO",      juce::Colour(0xFF226633));
    drawPanel(g, {698, top, 138, h}, "FILTER",   juce::Colour(0xFF662266));
    drawPanel(g, {840, top, 138, h}, "OUTPUT",   juce::Colour(0xFF445566));

    // Envelope panel section separators (match resized() geometry: eKH=60, eSep=8, eGap=4)
    const int panTop = top + 22;
    const int eKH = 60, eGap = 4, eSep = 8;
    int sepY1 = panTop + 2 + eKH + eGap;   // bottom of amp env row
    int sepY2 = sepY1 + eSep + eKH + eGap; // bottom of mod env row
    g.setColour(juce::Colour(0xFF226644).withAlpha(0.6f));
    g.drawLine(292, (float)sepY1, 548, (float)sepY1, 1.0f);
    g.setColour(juce::Colour(0xFFAAAAAA));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.drawText("MOD ENV", 294, sepY1 + 1, 80, 12, juce::Justification::centredLeft, false);
    g.setColour(juce::Colour(0xFF226644).withAlpha(0.6f));
    g.drawLine(292, (float)sepY2, 548, (float)sepY2, 1.0f);
    g.setColour(juce::Colour(0xFFAAAAAA));
    g.drawText("AMOUNT", 294, sepY2 + 1, 80, 12, juce::Justification::centredLeft, false);
}

// ============================================================
//  resized
// ============================================================
void CricketAudioProcessorEditor::resized()
{
    const int top    = 36;
    const int kH     = 76;   // knob height
    const int kW     = 60;   // knob width
    const int cbH    = 38;   // combo row height
    const int swH    = 54;   // toggle switch height
    const int rowGap = 4;
    const int panTop = top + 22; // below panel title bar

    // ---- LED chase strip ----
    ledChase.setBounds(4, top + 322, 974, 20);

    // ---- Title bar preset name label ----
    labelPreset.setBounds(162, 10, getWidth() - 360, 14);  // leave room for branding

    // ---- OSC 1 (x=4, w=138) ----
    {
        int x = 10, y = panTop + 2;
        waveOsc1.setBounds    (x, y, 126, cbH);          y += cbH + rowGap;
        knobOsc1Pitch.setBounds(x,       y, kW, kH);
        knobFMDepth.setBounds  (x+kW+4,  y, kW, kH);     y += kH + rowGap;
        knobOsc1Bend.setBounds (x + 33,  y, kW, kH);
    }

    // ---- OSC 2 (x=146, w=138) ----
    {
        int x = 152, y = panTop + 2;
        waveOsc2.setBounds    (x, y, 126, cbH);          y += cbH + rowGap;
        knobOsc2Pitch.setBounds(x,       y, kW, kH);
        knobMix.setBounds      (x+kW+4,  y, kW, kH);     y += kH + rowGap;
        knobOsc2Bend.setBounds (x + 33,  y, kW, kH);     y += kH + rowGap + 4;
        btnOsc2Range.setBounds (x + 22,  y, 82, swH);
    }

    // ---- ENVELOPE (x=288, w=264) ----
    {
        const int ex   = 294;
        const int eGap = 4;
        const int eKW  = kW;
        const int eKH  = 60;
        const int eSep = 8;

        int y = panTop + 2;

        knobAttack.setBounds (ex,              y, eKW, eKH);
        knobDecay.setBounds  (ex+eKW+eGap,     y, eKW, eKH);
        knobSustain.setBounds(ex+(eKW+eGap)*2, y, eKW, eKH);
        knobRelease.setBounds(ex+(eKW+eGap)*3, y, eKW, eKH);
        y += eKH + eGap + eSep;

        knobModA.setBounds(ex,              y, eKW, eKH);
        knobModD.setBounds(ex+eKW+eGap,     y, eKW, eKH);
        knobModS.setBounds(ex+(eKW+eGap)*2, y, eKW, eKH);
        knobModR.setBounds(ex+(eKW+eGap)*3, y, eKW, eKH);
        y += eKH + eGap + eSep;

        // Mod amount row 1: osc1, osc2, FM, cutoff
        knobModOsc1.setBounds (ex,              y, eKW, eKH);
        knobModOsc2.setBounds (ex+eKW+eGap,     y, eKW, eKH);
        knobModFM.setBounds   (ex+(eKW+eGap)*2, y, eKW, eKH);
        knobModCutoff.setBounds(ex+(eKW+eGap)*3,y, eKW, eKH);
        y += eKH + eGap;

        // Mod amount row 2: LFO speed, wrap + loop switch
        knobModLFO.setBounds (ex,          y, eKW, eKH);
        knobModWrap.setBounds(ex+eKW+eGap, y, eKW, eKH);
        btnModLoop.setBounds (ex+(eKW+eGap)*2, y + (eKH - swH) / 2, 86, swH);
    }

    // ---- LFO (x=556, w=138) ----
    {
        int x = 562, y = panTop + 2;
        waveLFO.setBounds     (x,       y, 126, cbH);    y += cbH + rowGap;
        knobLFOSpeed.setBounds(x,       y, kW,  kH);
        knobLFODepth.setBounds(x+kW+4,  y, kW,  kH);    y += kH + rowGap;
        knobLFOCutoff.setBounds(x,      y, kW,  kH);
        knobLFORes.setBounds   (x+kW+4, y, kW,  kH);
    }

    // ---- FILTER (x=698, w=138) ----
    {
        int x = 704, y = panTop + 2;
        filterTypeCombo.setBounds(x, y, 126, cbH);       y += cbH + rowGap;
        knobCutoff.setBounds    (x,       y, kW, kH);
        knobFilterRes.setBounds (x+kW+4,  y, kW, kH);
    }

    // ---- OUTPUT (x=840, w=138) ----
    {
        int x = 846, y = panTop + 2;
        knobMasterVol.setBounds(x,       y, kW, kH);
        knobVelSens.setBounds  (x+kW+4,  y, kW, kH);    y += kH + rowGap;
        knobWrap.setBounds     (x + 33,  y, kW, kH);    y += kH + rowGap + 8;
        btnLogicOn.setBounds   (x,       y, 86, swH);   y += swH + rowGap;
        switchLogicMode.setBounds(x,     y, 86, cbH);
    }
}
