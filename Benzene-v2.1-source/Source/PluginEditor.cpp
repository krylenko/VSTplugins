#include "PluginEditor.h"

// ============================================================
//  BenzeneLookAndFeel
// ============================================================
BenzeneLookAndFeel::BenzeneLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xFF2B2A26));
    setColour (juce::Label::textColourId,                 juce::Colour (0xFFD8D2C2));
    setColour (juce::ComboBox::backgroundColourId,        juce::Colour (0xFF1E1D1A));
    setColour (juce::ComboBox::textColourId,              juce::Colour (0xFFD8D2C2));
    setColour (juce::ComboBox::outlineColourId,           juce::Colour (0xFF5A574E));
    setColour (juce::PopupMenu::backgroundColourId,       juce::Colour (0xFF1E1D1A));
    setColour (juce::PopupMenu::textColourId,             juce::Colour (0xFFD8D2C2));
}

void BenzeneLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int w, int h, float pos, float startAng, float endAng, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (5.0f);
    float radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    float cx = b.getCentreX(), cy = b.getCentreY();
    float ang = startAng + pos * (endAng - startAng);

    // bakelite-style dark dial with a brass rim
    g.setColour (juce::Colour (0xFF3A3833));
    g.fillEllipse (cx - radius, cy - radius, radius * 2, radius * 2);
    g.setColour (s.findColour (juce::Slider::rotarySliderFillColourId).withAlpha (0.9f));
    g.drawEllipse (cx - radius, cy - radius, radius * 2, radius * 2, 2.0f);

    // value arc
    juce::Path arc;
    arc.addArc (cx - radius, cy - radius, radius * 2, radius * 2, startAng, ang, true);
    g.setColour (s.findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (arc, juce::PathStrokeType (2.5f));

    // pointer
    float pr = radius * 0.72f;
    float px = cx + std::cos (ang - juce::MathConstants<float>::halfPi) * pr;
    float py = cy + std::sin (ang - juce::MathConstants<float>::halfPi) * pr;
    g.setColour (juce::Colour (0xFFEDE6D4));
    g.drawLine (cx, cy, px, py, 2.5f);
    g.fillEllipse (cx - 3, cy - 3, 6, 6);
}

void BenzeneLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h,
    bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 2.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);
    juce::Path tri;
    float ax = w - 14.0f, ay = h * 0.5f;
    tri.addTriangle (ax, ay - 3, ax + 6, ay - 3, ax + 3, ay + 3);
    g.setColour (box.findColour (juce::ComboBox::textColourId));
    g.fillPath (tri);
}

juce::Font BenzeneLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold);
}

// ============================================================
//  LabelledKnob
// ============================================================
LabelledKnob::LabelledKnob (const juce::String& name, juce::Colour col) : ring (col)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
    slider.setColour (juce::Slider::rotarySliderFillColourId, col);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xFFD8D2C2));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colour (0xFFB8B2A2));
    addAndMakeVisible (label);
}

void LabelledKnob::setValueSuffix (const juce::String& s) { slider.setTextValueSuffix (s); }

void LabelledKnob::resized()
{
    auto b = getLocalBounds();
    label.setBounds (b.removeFromTop (14));
    slider.setBounds (b);
}

// ============================================================
//  MiniKnob
// ============================================================
MiniKnob::MiniKnob (juce::Colour col)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, col);
    slider.setDoubleClickReturnValue (true, 0.0);   // double-click resets to 0
    addAndMakeVisible (slider);
}

void MiniKnob::resized() { slider.setBounds (getLocalBounds()); }


// ============================================================
//  Panel helper
// ============================================================
static void drawPanel (juce::Graphics& g, juce::Rectangle<int> r,
                       const juce::String& title, juce::Colour accent)
{
    g.setColour (juce::Colour (0xFF34322D));
    g.fillRoundedRectangle (r.toFloat(), 4.0f);
    g.setColour (accent.withAlpha (0.5f));
    g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 4.0f, 1.2f);

    auto bar = r.removeFromTop (20).toFloat();
    g.setColour (accent.withAlpha (0.22f));
    g.fillRoundedRectangle (bar, 3.0f);
    g.setColour (juce::Colour (0xFFEDE6D4));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
    g.drawText (title, bar.toNearestInt().reduced (6, 0),
                juce::Justification::centredLeft, false);
}

// ============================================================
//  Editor
// ============================================================
BenzeneAudioProcessorEditor::BenzeneAudioProcessorEditor (BenzeneAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&laf);

    carrierWave.addItemList (juce::StringArray { "Sine", "Triangle", "Sawtooth", "Pulse", "White", "Pink", "LFSR" }, 1);
    addAndMakeVisible (carrierWave);
    diodeType.addItemList (juce::StringArray { "Germanium", "Silicon" }, 1);
    addAndMakeVisible (diodeType);
    stability.addItemList (juce::StringArray { "Raw (1x)", "Gritty (2x)", "Smooth (4x)", "Clean (8x)" }, 1);
    addAndMakeVisible (stability);

    for (auto* k : { &knobFreq, &knobDrive, &knobDrift, &knobSym, &knobImbal, &knobInstab,
                     &knobInLvl, &knobCarLvl, &knobMix, &knobOut })
        addAndMakeVisible (*k);

    knobFreq.slider.setTextValueSuffix (" Hz");

    freqReadout.setJustificationType (juce::Justification::centredRight);
    freqReadout.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    freqReadout.setColour (juce::Label::textColourId, juce::Colour (0xFF8FB88F));
    addAndMakeVisible (freqReadout);

    auto& v = processor.apvts;
    aFreq   = std::make_unique<SAtt> (v, ParamID::carrierFreq,  knobFreq.slider);
    aDrive  = std::make_unique<SAtt> (v, ParamID::carrierDrive, knobDrive.slider);
    aDrift  = std::make_unique<SAtt> (v, ParamID::carrierDrift, knobDrift.slider);
    aSym    = std::make_unique<SAtt> (v, ParamID::symmetry,     knobSym.slider);
    aImbal  = std::make_unique<SAtt> (v, ParamID::imbalance,    knobImbal.slider);
    aInstab = std::make_unique<SAtt> (v, ParamID::instability,  knobInstab.slider);
    aInLvl  = std::make_unique<SAtt> (v, ParamID::inputLevel,   knobInLvl.slider);
    aCarLvl = std::make_unique<SAtt> (v, ParamID::carrierLevel, knobCarLvl.slider);
    aMix    = std::make_unique<SAtt> (v, ParamID::mix,          knobMix.slider);
    aOut    = std::make_unique<SAtt> (v, ParamID::outputLevel,  knobOut.slider);
    addAndMakeVisible (gateButton);
    gateButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xFFD8D2C2));
    aGate   = std::make_unique<BAtt> (v, ParamID::gateEnable,   gateButton);
    aWave   = std::make_unique<CAtt> (v, ParamID::carrierWave,  carrierWave);
    aDiode  = std::make_unique<CAtt> (v, ParamID::diodeType,    diodeType);
    aStab   = std::make_unique<CAtt> (v, ParamID::oversample,   stability);

    // ---- modulation source controls ----
    for (auto* k : { &lfoRate, &lfoBlend, &pitchSens, &envSens, &envSpeed })
        addAndMakeVisible (*k);
    aLfoRate  = std::make_unique<SAtt> (v, ParamID::lfoRate,   lfoRate.slider);
    aLfoBlend = std::make_unique<SAtt> (v, ParamID::lfoBlend,  lfoBlend.slider);
    aPitchSens= std::make_unique<SAtt> (v, ParamID::pitchSens, pitchSens.slider);
    aEnvSens  = std::make_unique<SAtt> (v, ParamID::envSens,   envSens.slider);
    aEnvSpeed = std::make_unique<SAtt> (v, ParamID::envSpeed,  envSpeed.slider);
    lfoRate.slider.setTextValueSuffix (" Hz");

    // ---- modulation depth matrix (3 sources x 8 destinations) ----
    const juce::Colour srcCol[3] = {
        juce::Colour (0xFF9C7FC9), juce::Colour (0xFF7FC99C), juce::Colour (0xFFC9B47F) };
    for (int s = 0; s < 3; ++s)
        for (int d = 0; d < 8; ++d)
        {
            depth[s][d] = std::make_unique<MiniKnob> (srcCol[s]);
            addAndMakeVisible (*depth[s][d]);
            aDepth[s][d] = std::make_unique<SAtt> (
                v, ParamID::modDepth (s, d), depth[s][d]->slider);
        }

    startTimerHz (15);

    // setSize last: it triggers resized(), which lays out every component —
    // they must all be constructed first (the depth-matrix knobs are created above).
    setSize (804, 500);
}

BenzeneAudioProcessorEditor::~BenzeneAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void BenzeneAudioProcessorEditor::timerCallback()
{
    float hz = processor.displayCarrierHz.load();
    juce::String txt = (hz >= 1000.0f)
        ? juce::String (hz / 1000.0f, 2) + " kHz"
        : juce::String (hz, 2) + " Hz";
    freqReadout.setText (txt, juce::dontSendNotification);

    // live modulation source activity for the strip indicators
    lfoLevel   = processor.dispLFO.load();
    pitchLevel = processor.dispPitch.load();
    envLevel   = processor.dispEnv.load();
    // repaint only the strip region (below the panels)
    repaint (0, 236, getWidth(), getHeight() - 236);
}

void BenzeneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF2B2A26));

    // title bar
    g.setColour (juce::Colour (0xFF211F1C));
    g.fillRect (0, 0, getWidth(), 30);
    g.setColour (juce::Colour (0xFFE8A33D));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 17.0f, juce::Font::bold));
    g.drawText ("BENZENE", 12, 4, 160, 22, juce::Justification::centredLeft, false);
    g.setColour (juce::Colour (0xFF8A857A));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
    g.drawText ("RING MODULATOR  v" + juce::String (JucePlugin_VersionString),
                120, 9, 220, 12, juce::Justification::centredLeft, false);

    const int top = 38, h = 200;
    drawPanel (g, { 8,   top, 332, h }, "SIGNAL GENERATOR", juce::Colour (0xFFE8A33D));
    drawPanel (g, { 348, top, 362, h }, "RING MODULATOR",   juce::Colour (0xFF7FB3C9));
    drawPanel (g, { 718, top,  78, h }, "OUT",              juce::Colour (0xFFB7B7A8));

    // ---- modulation strips ----
    const char* srcName[3]  = { "LFO", "PITCH TRACKER", "ENVELOPE" };
    const juce::Colour srcCol[3] = {
        juce::Colour (0xFF9C7FC9), juce::Colour (0xFF7FC99C), juce::Colour (0xFFC9B47F) };
    const float srcVal[3] = { lfoLevel, pitchLevel, envLevel };
    const int destX[8] = { 14, 96, 178, 256, 354, 436, 518, 600 };
    const char* destName[8] = { "FREQ","DRIVE","DRIFT","SYM","IMBAL","INST","INPUT","CARR" };
    const int stripH = 78;
    int sy = 252;

    for (int s = 0; s < 3; ++s)
    {
        juce::Rectangle<int> r (8, sy, 788, stripH - 4);
        g.setColour (juce::Colour (0xFF34322D));
        g.fillRoundedRectangle (r.toFloat(), 4.0f);
        g.setColour (srcCol[s].withAlpha (0.5f));
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 4.0f, 1.2f);

        // source name
        g.setColour (srcCol[s]);
        g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
        g.drawText (srcName[s], 16, sy + 5, 150, 14, juce::Justification::centredLeft, false);

        // per-destination lights: lit when that depth knob is non-zero, brightness
        // scaled by |source value * depth| so it pulses with actual modulation.
        // A more saturated version of the source colour is used so the lights pop.
        juce::Colour litCol = srcCol[s].withSaturation (1.0f).brighter (0.4f);
        const int mkSize = 34;
        int ky = sy + 30;
        for (int d = 0; d < 8; ++d)
        {
            float dep = processor.dispDepth[s][d].load();
            int kx = destX[d] + (78 - mkSize) / 2;
            float lx = (float) (kx + mkSize + 2);
            float lyc = (float) (ky + mkSize * 0.5f);
            if (std::fabs (dep) > 0.001f)
            {
                float amt = juce::jlimit (0.0f, 1.0f, std::fabs (dep * srcVal[s]));
                // soft glow halo behind the dot
                g.setColour (litCol.withAlpha (0.20f + 0.40f * amt));
                g.fillEllipse (lx - 2.0f, lyc - 6.0f, 12.0f, 12.0f);
                // bright core, always clearly lit (floor raised) and saturated
                g.setColour (litCol.withAlpha (0.65f + 0.35f * amt));
                g.fillEllipse (lx, lyc - 4.0f, 8.0f, 8.0f);
            }
            else
            {
                // dim unlit marker so the slot is visible but clearly off
                g.setColour (juce::Colour (0xFF44423C));
                g.fillEllipse (lx + 1.0f, lyc - 2.0f, 4.0f, 4.0f);
            }
        }
        sy += stripH;
    }

    // destination column headers: drawn once, in the gap between the panels and
    // the first strip (panels end at y=238, first strip starts at y=252).
    g.setColour (juce::Colour (0xFFB8B2A2));
    g.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
    for (int d = 0; d < 8; ++d)
        g.drawText (destName[d], destX[d], 240, 78, 11,
                    juce::Justification::centred, false);
}

void BenzeneAudioProcessorEditor::resized()
{
    const int top = 38;
    const int kw = 78, kh = 86;

    // Signal Generator
    carrierWave.setBounds (18, top + 26, 120, 22);
    freqReadout.setBounds (148, top + 26, 184, 22);
    knobFreq.setBounds  (14,  top + 54, kw, kh);
    knobDrive.setBounds (96,  top + 54, kw, kh);
    knobDrift.setBounds (178, top + 54, kw, kh);
    knobSym.setBounds   (256, top + 54, kw, kh);

    // Ring Modulator
    diodeType.setBounds (358, top + 26, 160, 22);
    stability.setBounds (524, top + 26, 178, 22);
    knobImbal.setBounds (354, top + 54, kw, kh);
    knobInstab.setBounds(436, top + 54, kw, kh);
    knobInLvl.setBounds (518, top + 54, kw, kh);
    knobCarLvl.setBounds(600, top + 54, kw, kh);

    // Output
    knobMix.setBounds (720, top + 26, 74, 74);
    knobOut.setBounds (720, top + 100, 74, 74);
    gateButton.setBounds (724, top + 178, 70, 20);

    // ---- modulation strips ----
    // 8 depth columns aligned under the destination knobs above.
    const int destX[8] = { 14, 96, 178, 256, 354, 436, 518, 600 };
    const int mkSize = 34;           // mini knob size
    const int stripH = 78;           // tall enough that source-control value text fits inside
    int sy = 252;                    // first strip top

    auto layoutStrip = [&] (int s, LabelledKnob* c1, LabelledKnob* c2)
    {
        int ky = sy + 30;            // depth-knob row y within strip
        // source controls sit in the OUT column on the far right, clear of the
        // 8th depth knob. Positioned so the value text below them stays inside
        // the strip body (sy .. sy+stripH-4).
        const int cx = 680, cw = 50, ch = 52;
        int cy = sy + 16;
        if (c1) c1->setBounds (cx,        cy, cw, ch);
        if (c2) c2->setBounds (cx + 56,   cy, cw, ch);
        // 8 depth knobs aligned under destination columns
        for (int d = 0; d < 8; ++d)
        {
            int kx = destX[d] + (78 - mkSize) / 2;   // centre in the 78px column
            depth[s][d]->setBounds (kx, ky, mkSize, mkSize);
        }
        sy += stripH;
    };

    layoutStrip (0, &lfoRate, &lfoBlend);   // LFO
    layoutStrip (1, &pitchSens, nullptr);   // Pitch
    layoutStrip (2, &envSens, &envSpeed);   // Env
}
