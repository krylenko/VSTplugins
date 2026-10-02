#include "PluginEditor.h"

// ============================================================
//  DragonflyLookAndFeel
// ============================================================
DragonflyLookAndFeel::DragonflyLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId,  juce::Colour(0xFF151515));
    setColour(juce::Slider::rotarySliderFillColourId,     juce::Colour(0xFF444444));
    setColour(juce::Slider::rotarySliderOutlineColourId,  juce::Colour(0xFF222222));
    setColour(juce::Slider::thumbColourId,                juce::Colour(0xFF00DDFF));
    setColour(juce::Label::textColourId,                  juce::Colour(0xFFCCCCCC));
    setColour(juce::ComboBox::backgroundColourId,         juce::Colour(0xFF242424));
    setColour(juce::ComboBox::textColourId,               juce::Colour(0xFFCCCCCC));
    setColour(juce::ComboBox::outlineColourId,            juce::Colour(0xFF444444));
    setColour(juce::PopupMenu::backgroundColourId,        juce::Colour(0xFF242424));
    setColour(juce::PopupMenu::textColourId,              juce::Colour(0xFFCCCCCC));
}

void DragonflyLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAng, float endAng, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(4.0f);
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float cx     = bounds.getCentreX();
    float cy     = bounds.getCentreY();

    g.setColour(juce::Colour(0xFF2A2A2A));
    g.fillEllipse(cx-radius, cy-radius, radius*2, radius*2);

    float arcR     = radius * 0.82f;
    bool isBipolar = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0);

    if (isBipolar) {
        float centre = (float)(-slider.getMinimum() / (slider.getMaximum()-slider.getMinimum()));
        float cAng   = startAng + (endAng-startAng)*centre;
        float vAng   = startAng + (endAng-startAng)*sliderPos;
        if (std::abs(sliderPos - centre) > 0.001f) {
            juce::Path arc;
            arc.addArc(cx-arcR, cy-arcR, arcR*2, arcR*2,
                       std::min(cAng,vAng), std::max(cAng,vAng), true);
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(arc, juce::PathStrokeType(3.0f));
        }
    } else {
        juce::Path arc;
        arc.addArc(cx-arcR, cy-arcR, arcR*2, arcR*2,
                   startAng, startAng + (endAng-startAng)*sliderPos, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(arc, juce::PathStrokeType(3.0f));
    }

    float kR = radius * 0.62f;
    g.setColour(juce::Colour(0xFF3C3C3C));
    g.fillEllipse(cx-kR, cy-kR, kR*2, kR*2);
    g.setColour(juce::Colour(0xFF555555));
    g.drawEllipse(cx-kR, cy-kR, kR*2, kR*2, 1.5f);

    float angle = startAng + (endAng-startAng)*sliderPos - juce::MathConstants<float>::halfPi;
    g.setColour(juce::Colour(0xFFEEEEEE));
    g.drawLine(cx, cy, cx + std::cos(angle)*kR*0.72f, cy + std::sin(angle)*kR*0.72f, 2.5f);
    g.fillEllipse(cx-2, cy-2, 4, 4);
}

void DragonflyLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h,
    bool, int, int, int, int, juce::ComboBox& box)
{
    auto b = juce::Rectangle<float>(0,0,(float)w,(float)h);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 1.0f);
    float ax = (float)w-14, ay = (float)h*0.5f;
    juce::Path arrow;
    arrow.addTriangle(ax, ay-3, ax+6, ay-3, ax+3, ay+3);
    g.setColour(box.findColour(juce::ComboBox::textColourId));
    g.fillPath(arrow);
}

void DragonflyLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& lbl)
{
    g.setColour(lbl.findColour(juce::Label::textColourId));
    g.setFont(getLabelFont(lbl));
    g.drawFittedText(lbl.getText(), lbl.getLocalBounds(),
                     lbl.getJustificationType(), 2);
}

juce::Font DragonflyLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold);
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
    label.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    addAndMakeVisible(label);
}

void LabelledKnob::resized()
{
    auto b = getLocalBounds();
    label.setBounds(b.removeFromBottom(14));
    slider.setBounds(b);
}

// ============================================================
//  Panel background helper
// ============================================================
static void drawPanelBg(juce::Graphics& g, juce::Rectangle<int> r,
                         const juce::String& title, juce::Colour accent)
{
    g.setColour(accent.withAlpha(0.12f));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(accent.withAlpha(0.50f));
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 6.0f, 1.0f);

    auto bar = r.removeFromTop(20).toFloat();
    g.setColour(accent.withAlpha(0.30f));
    g.fillRoundedRectangle(bar, 5.0f);
    g.setColour(juce::Colour(0xFFDDDDDD));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText(title, bar.toNearestInt().reduced(6, 0),
               juce::Justification::centredLeft, true);
}

// ============================================================
//  FXBlockPanel
// ============================================================
static const juce::Colour FX_ACCENTS[NUM_FX_BLOCKS] = {
    juce::Colour(0xFF3366CC),
    juce::Colour(0xFF33AA77),
    juce::Colour(0xFFAA6633),
    juce::Colour(0xFF993399)
};

static const char BLOCK_LETTERS[NUM_FX_BLOCKS] = { 'A', 'B', 'C', 'D' };

FXBlockPanel::FXBlockPanel(int idx) : blockIndex(idx)
{
    // v1.8: Bypass is no longer a type — it has its own checkbox below.
    typeCombo.addItem("MS20 LP",   1);
    typeCombo.addItem("MS20 HP",   2);
    typeCombo.addItem("Comb",      3);
    typeCombo.addItem("Chebyshev", 4);
    typeCombo.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(typeCombo);

    bypassBtn.setColour(juce::ToggleButton::textColourId,  juce::Colour(0xFFBBBBBB));
    bypassBtn.setColour(juce::ToggleButton::tickColourId,  FX_ACCENTS[idx].brighter(0.5f));
    bypassBtn.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xFF666666));
    addAndMakeVisible(bypassBtn);

    knobP0.slider.setColour(juce::Slider::rotarySliderFillColourId,
                            FX_ACCENTS[idx].brighter(0.2f));
    knobP1.slider.setColour(juce::Slider::rotarySliderFillColourId,
                            FX_ACCENTS[idx].withRotatedHue(0.12f).brighter(0.2f));
    addAndMakeVisible(knobP0);
    addAndMakeVisible(knobP1);

    refreshKnobLabels(0);
}

void FXBlockPanel::refreshKnobLabels(int typeChoice)
{
    struct Labels { const char* p0; const char* p1; };
    static const Labels tbl[] = {
        { "CUTOFF",  "RES"      },   // MS20 LP
        { "CUTOFF",  "RES"      },   // MS20 HP
        { "FREQ",    "FEEDBACK" },   // Comb
        { "DRIVE",   "ORDER"    },   // Chebyshev
    };
    int ti = std::max(0, std::min(3, typeChoice));
    knobP0.label.setText(tbl[ti].p0, juce::dontSendNotification);
    knobP1.label.setText(tbl[ti].p1, juce::dontSendNotification);
}

void FXBlockPanel::setBypassDim(bool bypassed)
{
    const float a = bypassed ? 0.35f : 1.0f;
    typeCombo.setAlpha(a);
    knobP0.setAlpha(a);
    knobP1.setAlpha(a);
}

void FXBlockPanel::paint(juce::Graphics& g)
{
    drawPanelBg(g, getLocalBounds(),
                juce::String("BLOCK ") + BLOCK_LETTERS[blockIndex],
                FX_ACCENTS[blockIndex]);
}

void FXBlockPanel::resized()
{
    const int pad = 6;
    int y = 22 + pad;
    int w = getWidth() - pad*2;
    typeCombo.setBounds(pad, y, w, 26);  y += 30;
    bypassBtn.setBounds(pad, y, w, 20);  y += 22;
    int kW = (w - pad) / 2;
    knobP0.setBounds(pad,        y, kW, 72);
    knobP1.setBounds(pad+kW+pad, y, kW, 72);
}

// ============================================================
//  Shared helper: populate a DepthSlot's combo and children
// ============================================================
static void initDepthSlot(DepthSlot& sl, int slotIdx, juce::Component* parent)
{
    sl.slotLabel.setText("DEST " + juce::String(slotIdx+1), juce::dontSendNotification);
    sl.slotLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                    8.5f, juce::Font::bold));
    sl.slotLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));
    sl.slotLabel.setJustificationType(juce::Justification::centredLeft);
    parent->addAndMakeVisible(sl.slotLabel);

    int defaultTypes[NUM_FX_BLOCKS] = {0,0,0,0};
    auto names = buildModTargetNames(defaultTypes);
    sl.targetCombo.clear(juce::dontSendNotification);
    sl.targetCombo.addItemList(names, 1);
    sl.targetCombo.setSelectedId(TARG_NONE + 1, juce::dontSendNotification);
    parent->addAndMakeVisible(sl.targetCombo);
    parent->addAndMakeVisible(sl.knobDepth);
}

static void refreshSlotTargetNames(DepthSlot& sl, const juce::StringArray& names)
{
    int currentId = sl.targetCombo.getSelectedId();
    sl.targetCombo.clear(juce::dontSendNotification);
    sl.targetCombo.addItemList(names, 1);
    if (currentId >= 1 && currentId <= names.size())
        sl.targetCombo.setSelectedId(currentId, juce::dontSendNotification);
    else
        sl.targetCombo.setSelectedId(TARG_NONE + 1, juce::dontSendNotification);
}

static void layoutDepthSlots(DepthSlot slots[], int numSlots,
                              int x, int& y, int w)
{
    for (int s = 0; s < numSlots; ++s)
    {
        auto& sl = slots[s];
        sl.slotLabel.setBounds(x, y, w, 14);       y += 15;
        sl.targetCombo.setBounds(x, y, w, 24);     y += 26;
        sl.knobDepth.setBounds(x + (w-52)/2, y, 52, 62);  y += 64;
        y += 2; // gap between slots
    }
}

// ============================================================
//  LFOPanel
// ============================================================
LFOPanel::LFOPanel(int idx) : lfoIndex(idx)
{
    waveCombo.addItem("Sawtooth", 1);
    waveCombo.addItem("Triangle", 2);
    waveCombo.addItem("Noise",    3);
    waveCombo.addItem("Square",   4);
    waveCombo.setSelectedId(2, juce::dontSendNotification);
    addAndMakeVisible(waveCombo);
    addAndMakeVisible(knobSpeed);

    for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
        initDepthSlot(slots[s], s, this);
}

void LFOPanel::refreshTargetNames(const int blockTypes[NUM_FX_BLOCKS])
{
    auto names = buildModTargetNames(blockTypes);
    for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
        refreshSlotTargetNames(slots[s], names);
}

void LFOPanel::paint(juce::Graphics& g)
{
    const juce::Colour accent = (lfoIndex == 0)
        ? juce::Colour(0xFF228855) : juce::Colour(0xFF885522);
    drawPanelBg(g, getLocalBounds(), "LFO " + juce::String(lfoIndex+1), accent);
}

void LFOPanel::resized()
{
    const int pad = 6;
    int y = 22 + pad;
    int w = getWidth() - pad*2;

    waveCombo.setBounds(pad, y, w, 24);   y += 28;
    knobSpeed.setBounds(pad + (w-56)/2, y, 56, 66);  y += 72;

    layoutDepthSlots(slots, LFO_DEPTH_SLOTS, pad, y, w);
}

// ============================================================
//  EnvPanel
// ============================================================
EnvPanel::EnvPanel()
{
    addAndMakeVisible(knobSens);
    addAndMakeVisible(knobSpeed);

    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
        initDepthSlot(slots[s], s, this);
}

void EnvPanel::refreshTargetNames(const int blockTypes[NUM_FX_BLOCKS])
{
    auto names = buildModTargetNames(blockTypes);
    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
        refreshSlotTargetNames(slots[s], names);
}

void EnvPanel::paint(juce::Graphics& g)
{
    drawPanelBg(g, getLocalBounds(), "ENV FOLLOW", juce::Colour(0xFFAA4422));
}

void EnvPanel::resized()
{
    const int pad = 6;
    int y = 22 + pad;
    int panW = getWidth() - pad*2;

    // Left section: two knobs side by side
    int kW = 56, kH = 66;
    knobSens.setBounds (pad, y, kW, kH);
    knobSpeed.setBounds(pad + kW + 4, y, kW, kH);

    // Right section: two depth slots side by side
    int slotX = pad + kW*2 + 16;
    int slotW = (getWidth() - slotX - pad - 6) / 2;

    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
    {
        auto& sl = slots[s];
        int sx = slotX + s * (slotW + 6);
        int sy = y;
        sl.slotLabel.setBounds   (sx, sy, slotW, 14);  sy += 15;
        sl.targetCombo.setBounds (sx, sy, slotW, 24);   sy += 26;
        sl.knobDepth.setBounds   (sx + (slotW-52)/2, sy, 52, 62);
    }
}

// ============================================================
//  Editor
// ============================================================
DragonflyFXAudioProcessorEditor::DragonflyFXAudioProcessorEditor(DragonflyFXAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&laf);
    setSize(1090, 404);

    // FX blocks
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        addAndMakeVisible(fxPanels[b]);
        attFxType  [b] = std::make_unique<CAtt>(p.apvts, ParamID::fxType(b),
                                                fxPanels[b].typeCombo);
        attFxBypass[b] = std::make_unique<BAtt>(p.apvts, ParamID::fxBypass(b),
                                                fxPanels[b].bypassBtn);
        attFxParam0[b] = std::make_unique<SAtt>(p.apvts, ParamID::fxParam0(b),
                                                fxPanels[b].knobP0.slider);
        attFxParam1[b] = std::make_unique<SAtt>(p.apvts, ParamID::fxParam1(b),
                                                fxPanels[b].knobP1.slider);
    }

    // LFOs
    for (int l = 0; l < NUM_LFOS; ++l)
    {
        addAndMakeVisible(lfoPanels[l]);
        attLfoWave [l] = std::make_unique<CAtt>(p.apvts, ParamID::lfoWave(l),
                                                lfoPanels[l].waveCombo);
        attLfoSpeed[l] = std::make_unique<SAtt>(p.apvts, ParamID::lfoSpeed(l),
                                                lfoPanels[l].knobSpeed.slider);
        for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
        {
            attLfoTarget[l][s] = std::make_unique<CAtt>(p.apvts,
                                     ParamID::lfoTarget(l, s),
                                     lfoPanels[l].slots[s].targetCombo);
            attLfoDepth [l][s] = std::make_unique<SAtt>(p.apvts,
                                     ParamID::lfoDepth(l, s),
                                     lfoPanels[l].slots[s].knobDepth.slider);
        }
    }

    // Envelope follower
    addAndMakeVisible(envPanel);
    attEnvSens  = std::make_unique<SAtt>(p.apvts, ParamID::envSens,
                                         envPanel.knobSens.slider);
    attEnvSpeed = std::make_unique<SAtt>(p.apvts, ParamID::envSpeed,
                                         envPanel.knobSpeed.slider);
    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
    {
        attEnvTarget[s] = std::make_unique<CAtt>(p.apvts, ParamID::envTarget(s),
                                                 envPanel.slots[s].targetCombo);
        attEnvDepth [s] = std::make_unique<SAtt>(p.apvts, ParamID::envDepth(s),
                                                 envPanel.slots[s].knobDepth.slider);
    }

    // Master / mix
    addAndMakeVisible(knobMix);
    attMix = std::make_unique<SAtt>(p.apvts, ParamID::mix, knobMix.slider);

    // Initial refresh
    {
        int currentTypes[NUM_FX_BLOCKS];
        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
            currentTypes[b] = (int)*processor.p_fxType[b];
        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
            fxPanels[b].refreshKnobLabels(currentTypes[b]);
        for (int l = 0; l < NUM_LFOS; ++l)
            lfoPanels[l].refreshTargetNames(currentTypes);
        envPanel.refreshTargetNames(currentTypes);
        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
            lastBlockTypes[b] = currentTypes[b];

        for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        {
            const int byp = *processor.p_fxBypass[b] > 0.5f ? 1 : 0;
            fxPanels[b].setBypassDim(byp != 0);
            lastBypass[b] = byp;
        }
    }

    startTimerHz(20);
}

DragonflyFXAudioProcessorEditor::~DragonflyFXAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void DragonflyFXAudioProcessorEditor::timerCallback()
{
    // Keep the bypass dimming in sync (checkbox, host automation, presets)
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        const int byp = *processor.p_fxBypass[b] > 0.5f ? 1 : 0;
        if (byp != lastBypass[b])
        {
            fxPanels[b].setBypassDim(byp != 0);
            lastBypass[b] = byp;
        }
    }

    int currentTypes[NUM_FX_BLOCKS];
    bool changed = false;
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
    {
        currentTypes[b] = (int)*processor.p_fxType[b];
        if (currentTypes[b] != lastBlockTypes[b]) changed = true;
    }
    if (!changed) return;

    // Update block knob labels
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        fxPanels[b].refreshKnobLabels(currentTypes[b]);

    // Detach all target combos
    for (int l = 0; l < NUM_LFOS; ++l)
        for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
            attLfoTarget[l][s].reset();
    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
        attEnvTarget[s].reset();

    // Refresh target names
    for (int l = 0; l < NUM_LFOS; ++l)
        lfoPanels[l].refreshTargetNames(currentTypes);
    envPanel.refreshTargetNames(currentTypes);

    // Re-attach
    for (int l = 0; l < NUM_LFOS; ++l)
        for (int s = 0; s < LFO_DEPTH_SLOTS; ++s)
            attLfoTarget[l][s] = std::make_unique<CAtt>(processor.apvts,
                                      ParamID::lfoTarget(l, s),
                                      lfoPanels[l].slots[s].targetCombo);
    for (int s = 0; s < ENV_DEPTH_SLOTS; ++s)
        attEnvTarget[s] = std::make_unique<CAtt>(processor.apvts,
                                  ParamID::envTarget(s),
                                  envPanel.slots[s].targetCombo);

    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        lastBlockTypes[b] = currentTypes[b];
}

void DragonflyFXAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF151515));

    // Title bar
    g.setColour(juce::Colour(0xFF1E1E1E));
    g.fillRect(0, 0, getWidth(), 34);
    g.setColour(juce::Colour(0xFF333333));
    g.drawLine(0.0f, 34.0f, (float)getWidth(), 34.0f, 1.0f);

    g.setColour(juce::Colour(0xFF00DDFF));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 18.0f, juce::Font::bold));
    g.drawText("DRAGONFLY FX", 12, 5, 160, 24, juce::Justification::centredLeft, false);

    g.setColour(juce::Colour(0xFF558866));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    g.drawText("v" DRAGONFLY_VERSION, 176, 12, 36, 12,
               juce::Justification::centredLeft, false);

    g.setColour(juce::Colour(0xFF555555));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    g.drawText("modular effects chain", 216, 12, 180, 12,
               juce::Justification::centredLeft, false);

    g.setColour(juce::Colour(0xFF7A9AA8));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText("MURMUR ENGINEERING", getWidth() - 192, 12, 180, 12,
               juce::Justification::centredRight, false);

    // Divider between FX+Env and LFO modulators
    const int margin  = 4;
    const int masterW = 110;
    int divX = 4 + NUM_FX_BLOCKS * 150 + 4;
    int masterX = getWidth() - margin - masterW;
    g.setColour(juce::Colour(0xFF333333));
    g.drawLine((float)divX, 38.0f, (float)divX, (float)getHeight() - 4.0f, 1.0f);
    g.setColour(juce::Colour(0xFF444444));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
    g.drawText("FX CHAIN \xe2\x86\x92", 8, 36, 100, 12, juce::Justification::centredLeft, false);
    g.drawText("\xe2\x86\x90 LFO MOD", divX + 6, 36, 100, 12, juce::Justification::centredLeft, false);

    // MASTER (mix) panel, far right
    juce::Rectangle<int> masterRect(masterX, 38, masterW, getHeight() - 38 - margin);
    drawPanelBg(g, masterRect, "MASTER", juce::Colour(0xFF339999));
}

void DragonflyFXAudioProcessorEditor::resized()
{
    const int top     = 38;
    const int margin  = 4;
    const int fxW     = 146;
    const int fxH     = 164;  // title + combo + bypass + 2 knobs (DC block checkbox removed in v1.10)
    const int masterW = 110;

    // FX blocks: top-left, 4 across
    for (int b = 0; b < NUM_FX_BLOCKS; ++b)
        fxPanels[b].setBounds(margin + b*(fxW+margin), top, fxW, fxH);

    // Envelope follower: below the FX blocks, spanning their width
    int envX = margin;
    int envY = top + fxH + margin;
    int envW = NUM_FX_BLOCKS * (fxW + margin) - margin;
    int envH = getHeight() - envY - margin;
    envPanel.setBounds(envX, envY, envW, envH);

    // MASTER column: far right, holds the global mix knob
    int masterX = getWidth() - margin - masterW;
    knobMix.setBounds(masterX + (masterW-72)/2, top + 40, 72, 84);

    // LFO panels: middle-right column, between FX chain and MASTER
    int modX = margin + NUM_FX_BLOCKS * (fxW + margin) + margin;
    int modH = getHeight() - top - margin;
    int modAvailW = masterX - margin - modX;
    int lfoW = (modAvailW - (NUM_LFOS-1)*margin) / NUM_LFOS;

    for (int l = 0; l < NUM_LFOS; ++l)
        lfoPanels[l].setBounds(modX + l*(lfoW+margin), top, lfoW, modH);
}
