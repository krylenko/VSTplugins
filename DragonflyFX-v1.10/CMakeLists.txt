cmake_minimum_required(VERSION 3.22)

project(DragonflyFXVST VERSION 1.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(/root/JUCE DragonflyFXJUCE)

juce_add_plugin(DragonflyFX
    VERSION                     1.10.0
    COMPANY_NAME                "Dragonfly FX"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    Dfly
    PLUGIN_CODE                 Dfx1
    FORMATS                     VST3
    PRODUCT_NAME                "Dragonfly FX"
    VST3_CATEGORIES             "Fx|Filter|Distortion"
)

juce_generate_juce_header(DragonflyFX)

target_sources(DragonflyFX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(DragonflyFX PRIVATE Source)

target_compile_definitions(DragonflyFX PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DragonflyFX PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(DragonflyFX PRIVATE
        JUCE_DSP_USE_SIMD=0
        JUCE_USE_SSE_INTRINSICS=0
    )
endif()
