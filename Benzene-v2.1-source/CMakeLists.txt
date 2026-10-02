cmake_minimum_required(VERSION 3.22)
project(Benzene VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to the JUCE checkout. Override with -DJUCE_DIR=/path/to/JUCE if needed.
set(JUCE_DIR "$ENV{HOME}/JUCE" CACHE PATH "Path to the JUCE 8 source tree")
add_subdirectory(${JUCE_DIR} BenzeneJUCE)

juce_add_plugin(Benzene
    VERSION                     2.1.0
    COMPANY_NAME                "Daniel Ford"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    Dfrd
    PLUGIN_CODE                 Bnzn
    FORMATS                     VST3
    PRODUCT_NAME                "Benzene"
    VST3_CATEGORIES             "Fx|Modulation|Distortion"
)

juce_generate_juce_header(Benzene)

target_sources(Benzene PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(Benzene PRIVATE Source)

target_compile_definitions(Benzene PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Benzene PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Benzene PRIVATE
        JUCE_DSP_USE_SIMD=0
        JUCE_USE_SSE_INTRINSICS=0
    )
endif()
