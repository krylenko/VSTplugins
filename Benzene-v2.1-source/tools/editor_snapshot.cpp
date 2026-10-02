// ============================================================
//  Benzene editor snapshot / smoke test
//
//  Instantiates the processor + editor headlessly and renders the
//  UI to a PNG. Catches editor-construction crashes and layout
//  regressions on Linux without needing a Windows host.
//
//  Build as a juce_add_gui_app against the same Source/ files, then:
//      xvfb-run -a ./EditorSnapshot
//  Produces editor_snapshot.png in the working directory.
// ============================================================
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdio>

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;

    BenzeneAudioProcessor proc;
    proc.prepareToPlay (44100.0, 512);

    // Light up a few modulation routings so the indicator lights render.
    auto setP = [&] (const char* id, float v) {
        if (auto* p = proc.apvts.getParameter (id)) p->setValueNotifyingHost (v);
    };
    setP ("mod_lfo_carrierFreq", 0.9f);
    setP ("mod_pitch_imbalance", 0.8f);
    setP ("mod_env_instability", 0.85f);
    proc.dispLFO.store (0.9f);
    proc.dispPitch.store (0.8f);
    proc.dispEnv.store (0.95f);
    proc.dispDepth[0][0].store (0.8f);
    proc.dispDepth[1][4].store (0.6f);
    proc.dispDepth[2][5].store (0.7f);

    auto* ed = proc.createEditor();
    if (ed == nullptr) { std::printf ("FAIL: no editor\n"); return 1; }

    const int w = ed->getWidth(), h = ed->getHeight();
    juce::Image img (juce::Image::RGB, w, h, true);
    juce::Graphics g (img);
    ed->paintEntireComponent (g, false);

    juce::File out (juce::File::getCurrentWorkingDirectory()
                        .getChildFile ("editor_snapshot.png"));
    juce::PNGImageFormat png;
    juce::FileOutputStream os (out);
    png.writeImageToStream (img, os);

    std::printf ("OK: %dx%d -> %s\n", w, h, out.getFullPathName().toRawUTF8());
    delete ed;
    return 0;
}
