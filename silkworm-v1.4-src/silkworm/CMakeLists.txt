cmake_minimum_required(VERSION 3.22)

project(SilkwormVST VERSION 1.4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to the JUCE checkout — override with -DJUCE_DIR=/path/to/JUCE
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "$ENV{HOME}/JUCE")
endif()
add_subdirectory(${JUCE_DIR} SilkwormJUCE)

juce_add_plugin(Silkworm
    VERSION                     1.4.0
    COMPANY_NAME                "Daniel Ford"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    Dfrd
    PLUGIN_CODE                 Slkw
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Silkworm"
    VST3_CATEGORIES             "Fx|Reverb"
)

juce_generate_juce_header(Silkworm)

target_sources(Silkworm PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(Silkworm PRIVATE Source)

target_compile_definitions(Silkworm PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Silkworm PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    # LTO disabled: clang-cl cross-compile causes undefined symbols with LTO
    # juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Windows cross-compile: disable explicit SIMD intrinsics
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Silkworm PRIVATE
        JUCE_DSP_USE_SIMD=0
        JUCE_USE_SSE_INTRINSICS=0
    )
endif()
