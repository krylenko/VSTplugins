// Minimal VST3 smoke-test host: load module, instantiate, process audio, check output.
#define INIT_CLASS_IID
#include <windows.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstmessage.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
typedef IPluginFactory* (PLUGIN_API *GetFactoryProc)();
typedef bool (PLUGIN_API *InitProc)();

int main(int argc, char** argv)
{
    HMODULE m = LoadLibraryA(argv[1]);
    if (!m) { printf("FAIL LoadLibrary err=%lu\n", GetLastError()); return 1; }
    if (auto init = (InitProc) GetProcAddress(m, "InitDll")) init();
    auto gf = (GetFactoryProc) GetProcAddress(m, "GetPluginFactory");
    IPluginFactory* f = gf ? gf() : nullptr;
    if (!f) { printf("FAIL no factory\n"); return 1; }
    PFactoryInfo fi; f->getFactoryInfo(&fi);
    printf("Factory vendor: %s, classes: %d\n", fi.vendor, f->countClasses());
    TUID cid{}; bool found = false;
    for (int i = 0; i < f->countClasses(); ++i) {
        PClassInfo ci; f->getClassInfo(i, &ci);
        printf("  class %d: %s [%s]\n", i, ci.name, ci.category);
        if (strcmp(ci.category, kVstAudioEffectClass) == 0) { memcpy(cid, ci.cid, sizeof(TUID)); found = true; }
    }
    if (!found) { printf("FAIL no audio effect class\n"); return 1; }

    IComponent* comp = nullptr;
    if (f->createInstance(cid, IComponent::iid, (void**)&comp) != kResultOk || !comp) { printf("FAIL createInstance\n"); return 1; }
    if (comp->initialize(nullptr) != kResultOk) { printf("FAIL initialize\n"); return 1; }
    IAudioProcessor* proc = nullptr;
    comp->queryInterface(IAudioProcessor::iid, (void**)&proc);
    if (!proc) { printf("FAIL no IAudioProcessor\n"); return 1; }

    TUID ctrlId; IEditController* ctrl = nullptr;
    if (comp->getControllerClassId(ctrlId) == kResultOk)
        f->createInstance(ctrlId, IEditController::iid, (void**)&ctrl);
    if (!ctrl) comp->queryInterface(IEditController::iid, (void**)&ctrl);
    if (ctrl) { ctrl->initialize(nullptr);
        IConnectionPoint *a=nullptr,*b=nullptr; comp->queryInterface(IConnectionPoint::iid,(void**)&a); ctrl->queryInterface(IConnectionPoint::iid,(void**)&b);
        if (a&&b) { a->connect(b); b->connect(a); }
        printf("Parameters: %d\n", ctrl->getParameterCount()); }

    SpeakerArrangement st = SpeakerArr::kStereo;
    proc->setBusArrangements(&st, 1, &st, 1);
    ProcessSetup ps{ kRealtime, kSample32, 512, 48000.0 };
    if (proc->setupProcessing(ps) != kResultOk) { printf("FAIL setupProcessing\n"); return 1; }
    comp->activateBus(kAudio, kInput, 0, true); comp->activateBus(kAudio, kOutput, 0, true);
    comp->setActive(true); proc->setProcessing(true);

    std::vector<float> inL(512), inR(512), outL(512), outR(512);
    float* ins[2] = { inL.data(), inR.data() }; float* outs[2] = { outL.data(), outR.data() };
    AudioBusBuffers ib{}, ob{}; ib.numChannels = 2; ib.channelBuffers32 = ins; ob.numChannels = 2; ob.channelBuffers32 = outs;
    ProcessContext ctx{}; ctx.sampleRate = 48000; ctx.tempo = 120; ctx.state = ProcessContext::kPlaying | ProcessContext::kTempoValid;
    ProcessData d{}; d.processMode = kRealtime; d.symbolicSampleSize = kSample32; d.numSamples = 512;
    d.numInputs = 1; d.numOutputs = 1; d.inputs = &ib; d.outputs = &ob; d.processContext = &ctx;

    double phase = 0, peak = 0, sumsq = 0; long n = 0; bool bad = false;
    for (int b = 0; b < 400; ++b) {           // ~4.3 s of 220 Hz sine
        for (int i = 0; i < 512; ++i) { float s = 0.5f * (float) std::sin(phase); phase += 2 * M_PI * 220 / 48000; inL[i] = inR[i] = s; }
        if (proc->process(d) != kResultOk) { printf("FAIL process\n"); return 1; }
        for (int i = 0; i < 512; ++i) for (float v : { outL[i], outR[i] }) {
            if (!std::isfinite(v)) bad = true; peak = std::fmax(peak, std::fabs(v)); sumsq += v * v; ++n; }
    }
    printf("Output peak %.4f, RMS %.4f, non-finite: %s\n", peak, std::sqrt(sumsq / n), bad ? "YES" : "no");
    proc->setProcessing(false); comp->setActive(false);
    if (ctrl) { ctrl->terminate(); ctrl->release(); }
    proc->release(); comp->terminate(); comp->release();
    if (auto ex = (InitProc) GetProcAddress(m, "ExitDll")) ex();
    printf(bad || peak == 0 ? "RESULT: FAIL\n" : "RESULT: PASS\n");
    return bad ? 1 : 0;
}
