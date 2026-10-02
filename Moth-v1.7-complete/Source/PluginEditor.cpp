#include "PluginEditor.h"

using namespace MothColours;

static juce::Font terminalFont(float h, bool bold = false) {
    return juce::Font(juce::Font::getDefaultMonospacedFontName(), h,
                      bold ? juce::Font::bold : juce::Font::plain);
}

// ============================================================
//  MothLookAndFeel
// ============================================================
MothLookAndFeel::MothLookAndFeel()
{
    setColour(juce::PopupMenu::backgroundColourId,        panelDark);
    setColour(juce::PopupMenu::textColourId,              textGreen);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, phosphorDim);
    setColour(juce::PopupMenu::highlightedTextColourId,   phosphor);
    setColour(juce::ComboBox::backgroundColourId,         panelBlack);
    setColour(juce::ComboBox::textColourId,               textGreen);
    setColour(juce::ComboBox::outlineColourId,            metalBronze);
    setColour(juce::ComboBox::arrowColourId,              amber);
    setColour(juce::Slider::textBoxOutlineColourId,       juce::Colours::transparentBlack);
}

void MothLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAng, float endAng, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(3.0f);
    float radius  = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();

    juce::Colour accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

    // ---- engraved well behind the dial ----
    g.setColour(panelBlack);
    g.fillEllipse(cx-radius, cy-radius, radius*2, radius*2);
    g.setColour(metalEdge);
    g.drawEllipse(cx-radius, cy-radius, radius*2, radius*2, 1.5f);

    // ---- tick ring (engraved graduations) ----
    int ticks = 11;
    for (int i = 0; i < ticks; ++i) {
        float t = (float)i / (float)(ticks - 1);
        float a = startAng + (endAng - startAng) * t;
        float r1 = radius * 0.99f;
        float r2 = radius * 0.86f;
        float sx = cx + std::sin(a) * r1;
        float sy = cy - std::cos(a) * r1;
        float ex = cx + std::sin(a) * r2;
        float ey = cy - std::cos(a) * r2;
        bool lit = t <= sliderPos + 0.0001f;
        g.setColour(lit ? accent.withAlpha(0.95f) : metalEdge);
        g.drawLine(sx, sy, ex, ey, lit ? 1.8f : 1.2f);
    }

    // ---- value arc, phosphor glow ----
    float arcR = radius * 0.80f;
    juce::Path arc;
    arc.addArc(cx-arcR, cy-arcR, arcR*2, arcR*2,
               startAng, startAng + (endAng-startAng)*sliderPos, true);
    g.setColour(accent.withAlpha(0.25f));
    g.strokePath(arc, juce::PathStrokeType(5.0f));   // glow
    g.setColour(accent);
    g.strokePath(arc, juce::PathStrokeType(2.0f));   // core

    // ---- machined knob body (radial bronze) ----
    float knobR = radius * 0.62f;
    juce::ColourGradient grad(metalBronzeHi, cx - knobR*0.4f, cy - knobR*0.5f,
                              metalEdge,      cx + knobR*0.6f, cy + knobR*0.7f, true);
    g.setGradientFill(grad);
    g.fillEllipse(cx-knobR, cy-knobR, knobR*2, knobR*2);
    g.setColour(panelBlack.withAlpha(0.6f));
    g.drawEllipse(cx-knobR, cy-knobR, knobR*2, knobR*2, 1.0f);

    // knurled rim notches
    for (int i = 0; i < 24; ++i) {
        float a = (float)i / 24.0f * juce::MathConstants<float>::twoPi;
        float r1 = knobR * 0.98f, r2 = knobR * 0.86f;
        g.setColour(metalEdge.withAlpha(0.5f));
        g.drawLine(cx+std::sin(a)*r2, cy-std::cos(a)*r2,
                   cx+std::sin(a)*r1, cy-std::cos(a)*r1, 0.6f);
    }

    // ---- pointer ----
    float ang = startAng + (endAng-startAng)*sliderPos;
    float px = cx + std::sin(ang) * knobR * 0.82f;
    float py = cy - std::cos(ang) * knobR * 0.82f;
    g.setColour(accent.brighter(0.4f));
    g.drawLine(cx, cy, px, py, 2.6f);
    g.setColour(accent);
    g.fillEllipse(px-2.4f, py-2.4f, 4.8f, 4.8f);
    // hub
    g.setColour(panelBlack);
    g.fillEllipse(cx-3, cy-3, 6, 6);
    g.setColour(accent.withAlpha(0.7f));
    g.drawEllipse(cx-3, cy-3, 6, 6, 1.0f);
}

void MothLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& btn, bool highlighted, bool)
{
    auto b = btn.getLocalBounds().toFloat();
    bool on = btn.getToggleState();

    // LED + "ENABLE" terminal label
    float ledR = 6.0f;
    float lx = b.getX() + ledR + 4.0f;
    float ly = b.getCentreY();

    // bezel
    g.setColour(panelBlack);
    g.fillEllipse(lx-ledR-2, ly-ledR-2, (ledR+2)*2, (ledR+2)*2);
    g.setColour(metalEdge);
    g.drawEllipse(lx-ledR-2, ly-ledR-2, (ledR+2)*2, (ledR+2)*2, 1.0f);

    juce::Colour lit = on ? phosphor : warningRed.withAlpha(0.35f);
    if (on) {
        g.setColour(phosphor.withAlpha(0.30f));
        g.fillEllipse(lx-ledR*1.8f, ly-ledR*1.8f, ledR*3.6f, ledR*3.6f); // glow
    }
    g.setColour(lit);
    g.fillEllipse(lx-ledR, ly-ledR, ledR*2, ledR*2);
    g.setColour(juce::Colours::white.withAlpha(on ? 0.5f : 0.2f));
    g.fillEllipse(lx-ledR*0.4f, ly-ledR*0.5f, ledR*0.6f, ledR*0.6f);

    g.setColour(on ? textGreen : amberDim);
    g.setFont(terminalFont(11.0f, true));
    g.drawText(on ? "ONLINE" : "BYPASS",
               juce::Rectangle<float>(lx+ledR+6, b.getY(), b.getWidth()-lx-ledR-6, b.getHeight()).toNearestInt(),
               juce::Justification::centredLeft, false);

    if (highlighted) {
        g.setColour(amber.withAlpha(0.25f));
        g.drawRoundedRectangle(b.reduced(1.0f), 3.0f, 1.0f);
    }
}

void MothLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h,
    bool, int, int, int, int, juce::ComboBox& box)
{
    auto b = juce::Rectangle<float>(0,0,(float)w,(float)h);
    g.setColour(panelBlack);
    g.fillRoundedRectangle(b, 2.0f);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(b.reduced(0.5f), 2.0f, 1.0f);

    // bracket-style arrow [▼]
    float ax = w - 16.0f, ay = h * 0.5f;
    juce::Path tri;
    tri.addTriangle(ax, ay-3, ax+8, ay-3, ax+4, ay+3);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.fillPath(tri);
}

void MothLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(6, 0, box.getWidth() - 24, box.getHeight());
    label.setFont(getComboBoxFont(box));
}

juce::Font MothLookAndFeel::getComboBoxFont(juce::ComboBox&) { return terminalFont(12.0f, true); }
juce::Font MothLookAndFeel::getLabelFont(juce::Label&)        { return terminalFont(11.0f, false); }

void MothLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int w, int h)
{
    g.fillAll(panelDark);
    g.setColour(metalBronze);
    g.drawRect(0, 0, w, h, 1);
}

// ============================================================
//  TerminalKnob
// ============================================================
TerminalKnob::TerminalKnob(const juce::String& name, juce::Colour col)
    : caption(name), accent(col)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour(juce::Slider::rotarySliderFillColourId, col);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                               juce::MathConstants<float>::pi * 2.75f, true);
    slider.setPopupDisplayEnabled(true, true, nullptr);
    addAndMakeVisible(slider);
}

void TerminalKnob::resized()
{
    auto b = getLocalBounds();
    b.removeFromBottom(16);  // caption plate
    slider.setBounds(b);
}

void TerminalKnob::paint(juce::Graphics& g)
{
    // engraved caption plate at bottom
    auto plate = getLocalBounds().removeFromBottom(15).toFloat().reduced(2.0f, 0.0f);
    g.setColour(panelBlack);
    g.fillRoundedRectangle(plate, 2.0f);
    g.setColour(metalEdge);
    g.drawRoundedRectangle(plate, 2.0f, 0.8f);
    g.setColour(accent.withAlpha(0.92f));
    g.setFont(terminalFont(9.5f, true));
    g.drawText(caption, plate.toNearestInt(), juce::Justification::centred, false);
}

// ============================================================
//  TerminalCombo
// ============================================================
TerminalCombo::TerminalCombo(const juce::String& name, juce::StringArray items)
    : caption(name)
{
    combo.addItemList(items, 1);
    combo.setSelectedId(1, juce::dontSendNotification);
    combo.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(combo);
}

void TerminalCombo::resized()
{
    auto b = getLocalBounds();
    b.removeFromTop(14);
    combo.setBounds(b.reduced(2, 0));
}

void TerminalCombo::paint(juce::Graphics& g)
{
    auto cap = getLocalBounds().removeFromTop(13).toFloat();
    g.setColour(textBronze.withAlpha(0.9f));
    g.setFont(terminalFont(9.5f, true));
    g.drawText(caption, cap.toNearestInt(), juce::Justification::centredLeft, false);
}

// ============================================================
//  CRTMeter
// ============================================================
CRTMeter::CRTMeter(MothAudioProcessor& p) : processor(p) { startTimerHz(30); }

void CRTMeter::timerCallback()
{
    float l = processor.outMeterL.load();
    float r = processor.outMeterR.load();
    // peak-hold with decay
    dispL = std::max(l, dispL * 0.82f);
    dispR = std::max(r, dispR * 0.82f);
    repaint();
}

void CRTMeter::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(panelBlack);
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(metalEdge);
    g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 1.0f);

    auto inner = b.reduced(5.0f);
    float rowH = inner.getHeight() * 0.5f - 2.0f;

    auto drawBar = [&](juce::Rectangle<float> row, float val, const char* lbl) {
        g.setColour(phosphorDim.withAlpha(0.35f));
        g.setFont(terminalFont(8.0f, true));
        g.drawText(lbl, row.removeFromLeft(14).toNearestInt(),
                   juce::Justification::centredLeft, false);
        // segmented bar
        int segs = 20;
        float segW = row.getWidth() / (float)segs;
        for (int i = 0; i < segs; ++i) {
            float t = (float)i / (float)(segs - 1);
            bool lit = val >= t;
            juce::Colour c = (t > 0.85f) ? warningRed : (t > 0.6f ? amber : phosphor);
            g.setColour(lit ? c : c.withAlpha(0.12f));
            g.fillRect(row.getX() + i*segW + 0.5f, row.getY()+1.0f, segW-1.2f, row.getHeight()-2.0f);
        }
    };

    auto top = inner.removeFromTop(rowH);
    inner.removeFromTop(4.0f);
    auto bot = inner.removeFromTop(rowH);
    drawBar(top, juce::jlimit(0.0f, 1.0f, dispL), "L");
    drawBar(bot, juce::jlimit(0.0f, 1.0f, dispR), "R");
}

// ============================================================
//  Editor
// ============================================================
MothAudioProcessorEditor::MothAudioProcessorEditor(MothAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), meter(p)
{
    setLookAndFeel(&laf);
    setSize(720, 408);

    addAndMakeVisible(distPower);
    addAndMakeVisible(knobDistortion);
    addAndMakeVisible(knobFilter);
    addAndMakeVisible(knobDistVol);

    addAndMakeVisible(phaserPower);
    addAndMakeVisible(knobSpeed);
    addAndMakeVisible(knobDepth);
    addAndMakeVisible(knobCharacter);
    addAndMakeVisible(knobFeedback);
    addAndMakeVisible(comboShape);

    addAndMakeVisible(knobMix);
    addAndMakeVisible(knobOut);
    addAndMakeVisible(meter);

    auto& v = processor.apvts;
    attDistOn     = std::make_unique<BAtt>(v, ParamID::distOn,     distPower);
    attDistortion = std::make_unique<SAtt>(v, ParamID::distortion, knobDistortion.slider);
    attFilter     = std::make_unique<SAtt>(v, ParamID::filter,     knobFilter.slider);
    attDistVol    = std::make_unique<SAtt>(v, ParamID::distVolume, knobDistVol.slider);

    attPhaserOn   = std::make_unique<BAtt>(v, ParamID::phaserOn,   phaserPower);
    attSpeed      = std::make_unique<SAtt>(v, ParamID::speed,      knobSpeed.slider);
    attDepth      = std::make_unique<SAtt>(v, ParamID::depth,      knobDepth.slider);
    attCharacter  = std::make_unique<SAtt>(v, ParamID::character,  knobCharacter.slider);
    attFeedback   = std::make_unique<SAtt>(v, ParamID::feedback,   knobFeedback.slider);
    attShape      = std::make_unique<CAtt>(v, ParamID::lfoShape,   comboShape.combo);

    attMix        = std::make_unique<SAtt>(v, ParamID::mix,        knobMix.slider);
    attOut        = std::make_unique<SAtt>(v, ParamID::outLevel,   knobOut.slider);

    startTimerHz(30);
}

MothAudioProcessorEditor::~MothAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void MothAudioProcessorEditor::timerCallback()
{
    scanPhase += 0.012f;
    if (scanPhase > 1.0f) scanPhase -= 1.0f;
    repaint();
}

void MothAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<int> r,
                                          const juce::String& title, bool enabled,
                                          float scanP)
{
    auto rf = r.toFloat();

    // panel base
    g.setColour(panelDark);
    g.fillRoundedRectangle(rf, 4.0f);

    // engraved double border (Dune bronze)
    g.setColour(enabled ? metalBronze : metalEdge);
    g.drawRoundedRectangle(rf.reduced(1.0f), 4.0f, 1.4f);
    g.setColour((enabled ? metalBronzeHi : metalEdge).withAlpha(0.4f));
    g.drawRoundedRectangle(rf.reduced(3.5f), 3.0f, 0.8f);

    // CRT scanlines inside the panel
    {
        juce::Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(r.reduced(4));
        g.setColour(phosphor.withAlpha(enabled ? 0.035f : 0.015f));
        for (float yy = rf.getY(); yy < rf.getBottom(); yy += 3.0f)
            g.drawHorizontalLine((int)yy, rf.getX(), rf.getRight());
        // a slow travelling brighter scan band
        float band = rf.getY() + scanP * rf.getHeight();
        g.setColour(phosphor.withAlpha(enabled ? 0.05f : 0.0f));
        g.fillRect(rf.getX(), band, rf.getWidth(), 14.0f);
    }

    // title bar plate
    auto tb = rf.removeFromTop(20.0f).reduced(4.0f, 3.0f);
    g.setColour(panelBlack);
    g.fillRoundedRectangle(tb, 2.0f);
    g.setColour(enabled ? amber : amberDim);
    g.setFont(terminalFont(12.0f, true));
    g.drawText(title, tb.reduced(6,0).toNearestInt(),
               juce::Justification::centredLeft, false);
    // status glyph on the right
    g.setColour(enabled ? phosphor : warningRed.withAlpha(0.6f));
    g.setFont(terminalFont(9.0f, true));
    g.drawText(enabled ? "[ACTIVE]" : "[ STBY ]",
               tb.reduced(6,0).toNearestInt(), juce::Justification::centredRight, false);
}

void MothAudioProcessorEditor::paint(juce::Graphics& g)
{
    // background: deep CRT black with subtle vignette
    g.fillAll(panelBlack);
    {
        juce::ColourGradient vg(juce::Colour(0xFF14180f), getWidth()*0.5f, getHeight()*0.4f,
                                panelBlack, 0, getHeight(), true);
        g.setGradientFill(vg);
        g.fillRect(getLocalBounds());
    }

    // ---- header bar ----
    auto header = juce::Rectangle<int>(0, 0, getWidth(), 40);
    g.setColour(panelDark);
    g.fillRect(header);
    g.setColour(metalBronze);
    g.drawHorizontalLine(40, 0.0f, (float)getWidth());
    g.setColour(metalBronzeHi.withAlpha(0.3f));
    g.drawHorizontalLine(41, 0.0f, (float)getWidth());

    // wordmark
    g.setColour(amber);
    g.setFont(terminalFont(22.0f, true));
    g.drawText("M O T H", 16, 6, 200, 28, juce::Justification::centredLeft, false);
    // moth glyph (simple wings) next to wordmark
    {
        float gx = 138.0f, gy = 20.0f;
        g.setColour(phosphor.withAlpha(0.85f));
        juce::Path wings;
        wings.addEllipse(gx-12, gy-7, 11, 14);
        wings.addEllipse(gx+1,  gy-7, 11, 14);
        g.fillPath(wings);
        g.setColour(panelBlack);
        g.fillEllipse(gx-1.5f, gy-8, 3, 16); // body
    }

    g.setColour(textBronze.withAlpha(0.7f));
    g.setFont(terminalFont(10.0f, false));
    g.drawText("DISTORTION // PHASE-SHIFT UNIT",
               220, 6, 340, 14, juce::Justification::centredLeft, false);
    g.setColour(phosphorDim);
    g.drawText("SER. MTH-" + juce::String(JucePlugin_VersionString).replace(".", ""),
               220, 22, 340, 12, juce::Justification::centredLeft, false);

    // manufacturer + version, small, right side
    g.setColour(textBronze.withAlpha(0.7f));
    g.setFont(terminalFont(10.0f, false));
    g.drawText("MURMUR ENGINEERING",
               getWidth()-240, 5, 226, 12, juce::Justification::centredRight, false);
    g.setColour(amber.withAlpha(0.85f));
    g.setFont(terminalFont(11.0f, true));
    g.drawText("v" + juce::String(JucePlugin_VersionString),
               getWidth()-70, 22, 56, 14, juce::Justification::centredRight, false);

    // ---- panels ----
    const int top = 50, bodyH = 300;
    drawPanel(g, {10,  top, 210, bodyH}, "RAT-616  OVERDRIVE",
              processor.apvts.getRawParameterValue(ParamID::distOn)->load() > 0.5f, scanPhase);
    drawPanel(g, {228, top, 330, bodyH}, "SCHULTE  PHASE-MATRIX",
              processor.apvts.getRawParameterValue(ParamID::phaserOn)->load() > 0.5f, scanPhase);
    drawPanel(g, {566, top, 144, bodyH}, "MASTER", true, scanPhase);

    // signal-flow arrows between panels
    g.setColour(phosphor.withAlpha(0.5f));
    g.setFont(terminalFont(14.0f, true));
    g.drawText(">", 219, top + bodyH/2 - 8, 10, 16, juce::Justification::centred, false);
    g.drawText(">", 557, top + bodyH/2 - 8, 10, 16, juce::Justification::centred, false);

    // ---- footer status strip ----
    auto footer = juce::Rectangle<int>(10, top + bodyH + 6, getWidth()-20, 36);
    g.setColour(panelDark);
    g.fillRoundedRectangle(footer.toFloat(), 3.0f);
    g.setColour(metalEdge);
    g.drawRoundedRectangle(footer.toFloat().reduced(0.5f), 3.0f, 1.0f);
}

void MothAudioProcessorEditor::resized()
{
    const int top = 50;

    // ---- Distortion panel (x=10 w=210) ----
    {
        int px = 10, py = top + 26;
        distPower.setBounds(px + 12, py, 180, 20);
        int ky = py + 28;
        knobDistortion.setBounds(px + 18,  ky, 84, 100);
        knobFilter.setBounds    (px + 108, ky, 84, 100);
        knobDistVol.setBounds   (px + 63,  ky + 104, 84, 100);
    }

    // ---- Phaser panel (x=228 w=330) ----
    {
        int px = 228, py = top + 26;
        phaserPower.setBounds(px + 12, py, 180, 20);
        int ky = py + 28;
        knobSpeed.setBounds    (px + 18,  ky, 84, 100);
        knobDepth.setBounds    (px + 122, ky, 84, 100);
        knobCharacter.setBounds(px + 226, ky, 84, 100);
        knobFeedback.setBounds (px + 70,  ky + 104, 84, 100);
        comboShape.setBounds   (px + 176, ky + 120, 128, 40);
    }

    // ---- Master panel (x=566 w=144) ----
    {
        int px = 566, py = top + 26;
        knobMix.setBounds(px + 30, py + 6,   84, 100);
        knobOut.setBounds(px + 30, py + 110, 84, 100);
        meter.setBounds  (px + 12, py + 214, 120, 52);
    }
}
