cmake_minimum_required(VERSION 3.22)

project(CricketVST VERSION 1.32.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to the JUCE checkout
add_subdirectory($ENV{HOME}/JUCE CricketJUCE)

juce_add_plugin(Cricket
    VERSION                     1.32.0
    COMPANY_NAME                "Daniel Ford"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    Dfrd
    PLUGIN_CODE                 Crkt
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Cricket"
    VST3_CATEGORIES             "Instrument|Synth|Drum"
)

juce_generate_juce_header(Cricket)

target_sources(Cricket PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/StandaloneApp.cpp
)

target_include_directories(Cricket PRIVATE Source)

target_compile_definitions(Cricket PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    # Enable ASIO on Windows (uses JUCE bundled headers, no external SDK needed)
    $<$<BOOL:${WIN32}>:JUCE_ASIO=1>
)

target_link_libraries(Cricket PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    # LTO disabled: clang-cl cross-compile causes SSE intrinsic undefined symbols with LTO
    # juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Windows cross-compile: disable explicit SIMD intrinsics
# (MSVC xmmintrin.h declares them as extern; clang-cl can't inline them cross-compiled)
# Cricket's synth engine is integer/float math that doesn't need explicit SSE.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Cricket PRIVATE
        JUCE_DSP_USE_SIMD=0
        JUCE_USE_SSE_INTRINSICS=0
    )
endif()
