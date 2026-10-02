// Cricket standalone application — custom JUCEApplication that passes
// ".ckt" as the preset file suffix to Save/Load state dialogs.
//
// Enabled by JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 in CMakeLists.txt.
// Compiled for all targets but only active when building standalone.

#include <JuceHeader.h>

#if JucePlugin_Build_Standalone

#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#include "PluginProcessor.h"

// ============================================================
//  CricketFilterWindow — overrides handleMenuResult to pass .ckt suffix
// ============================================================
class CricketFilterWindow : public juce::StandaloneFilterWindow
{
public:
    CricketFilterWindow (const juce::String& title,
                         juce::Colour backgroundColour,
                         std::unique_ptr<juce::StandalonePluginHolder> holder)
        : StandaloneFilterWindow (title, backgroundColour, std::move (holder))
    {}

    void handleMenuResult (int result) override
    {
        switch (result)
        {
            case 1:
                pluginHolder->showAudioSettingsDialog();
                break;

            case 2:
                if (auto* proc = dynamic_cast<CricketAudioProcessor*> (
                        pluginHolder->processor.get()))
                {
                    juce::File startDir = proc->lastPresetFolder.isDirectory()
                        ? proc->lastPresetFolder
                        : juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);

                    // Patch the holder's lastStateFile location so askUserToSaveState
                    // opens in the right folder, then delegate normally.
                    // Simplest: just override with our own chooser like the load path.
                    stateFileChooser = std::make_unique<juce::FileChooser> (
                        "Save current state", startDir, "*.ckt");

                    auto flags = juce::FileBrowserComponent::saveMode
                               | juce::FileBrowserComponent::canSelectFiles
                               | juce::FileBrowserComponent::warnAboutOverwriting;

                    stateFileChooser->launchAsync (flags, [this, proc] (const juce::FileChooser& fc)
                    {
                        auto result = fc.getResult();
                        if (result == juce::File{}) return;

                        juce::MemoryBlock data;
                        proc->getStateInformation (data);
                        result.replaceWithData (data.getData(), data.getSize());
                        proc->presetName       = result.getFileNameWithoutExtension();
                        proc->lastPresetFolder = result.getParentDirectory();
                    });
                }
                break;

            case 3:
                if (auto* proc = dynamic_cast<CricketAudioProcessor*> (
                        pluginHolder->processor.get()))
                {
                    // Use the last preset folder if set, otherwise Documents
                    juce::File startDir = proc->lastPresetFolder.isDirectory()
                        ? proc->lastPresetFolder
                        : juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);

                    stateFileChooser = std::make_unique<juce::FileChooser> (
                        "Load a saved state", startDir, "*.ckt");

                    auto flags = juce::FileBrowserComponent::openMode
                               | juce::FileBrowserComponent::canSelectFiles;

                    stateFileChooser->launchAsync (flags, [this, proc] (const juce::FileChooser& fc)
                    {
                        auto result = fc.getResult();
                        if (result == juce::File{}) return;

                        juce::MemoryBlock data;
                        if (result.loadFileAsData (data))
                        {
                            proc->allowStateRestore.store (true);
                            proc->setStateInformation (data.getData(), (int) data.getSize());
                            proc->allowStateRestore.store (false);
                            proc->presetName       = result.getFileNameWithoutExtension();
                            proc->lastPresetFolder = result.getParentDirectory();
                        }
                    });
                }
                break;

            case 4:
                if (auto* proc = dynamic_cast<CricketAudioProcessor*> (
                        pluginHolder->processor.get()))
                    proc->presetName = {};
                resetToDefaultState();
                break;

            default:
                break;
        }
    }

private:
    std::unique_ptr<juce::FileChooser> stateFileChooser;
};

// ============================================================
//  CricketStandaloneApp
// ============================================================
class CricketStandaloneApp : public juce::JUCEApplication
{
public:
    CricketStandaloneApp()
    {
        juce::PropertiesFile::Options options;
        options.applicationName     = juce::CharPointer_UTF8 (JucePlugin_Name);
        options.filenameSuffix      = ".settings";
        options.osxLibrarySubFolder = "Application Support";
       #if JUCE_LINUX || JUCE_BSD
        options.folderName          = "~/.config";
       #else
        options.folderName          = "";
       #endif
        appProperties.setStorageParameters (options);
    }

    const juce::String getApplicationName()    override { return JucePlugin_Name; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed()          override { return true; }
    void anotherInstanceStarted (const juce::String&) override {}

    std::unique_ptr<juce::StandalonePluginHolder> createPluginHolder()
    {
        const juce::Array<juce::StandalonePluginHolder::PluginInOuts> channelConfig;

        constexpr auto autoOpenMidi =
           #if (JUCE_ANDROID || JUCE_IOS) && ! JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                true;
           #else
                false;
           #endif

        return std::make_unique<juce::StandalonePluginHolder> (
            appProperties.getUserSettings(),
            false,
            juce::String{},
            nullptr,
            channelConfig,
            autoOpenMidi);
    }

    CricketFilterWindow* createWindow()
    {
        return new CricketFilterWindow (
            getApplicationName(),
            juce::LookAndFeel::getDefaultLookAndFeel()
                .findColour (juce::ResizableWindow::backgroundColourId),
            createPluginHolder());
    }

    void initialise (const juce::String&) override
    {
        mainWindow.reset (createWindow());
        if (mainWindow != nullptr)
            mainWindow->setVisible (true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        appProperties.saveIfNeeded();
    }

    void systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
            mainWindow->pluginHolder->savePluginState();

        if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            juce::Timer::callAfterDelay (100, []() {
                if (auto* app = juce::JUCEApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

private:
    juce::ApplicationProperties appProperties;
    std::unique_ptr<CricketFilterWindow> mainWindow;
};

// ============================================================
//  Entry point
// ============================================================
START_JUCE_APPLICATION (CricketStandaloneApp)

#endif // JucePlugin_Build_Standalone
