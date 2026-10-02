cmake_minimum_required(VERSION 3.22)

project(MothVST VERSION 1.7.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use ccache when available so JUCE module objects are served from cache
# across reconfigures (e.g. version bumps), making iterations fast.
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# Point to the JUCE checkout
add_subdirectory($ENV{HOME}/JUCE MothJUCE)

juce_add_plugin(Moth
    VERSION                     1.7.0
    COMPANY_NAME                "Daniel Ford"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    Dfrd
    PLUGIN_CODE                 Moth
    FORMATS                     VST3
    PRODUCT_NAME                "Moth"
    VST3_CATEGORIES             "Fx|Distortion|Modulation"
)

juce_generate_juce_header(Moth)

target_sources(Moth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(Moth PRIVATE Source)

target_compile_definitions(Moth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Moth PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Windows cross-compile: disable explicit SIMD intrinsics
# (MSVC xmmintrin.h declares them as extern; clang-cl can't inline them cross-compiled)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Moth PRIVATE
        JUCE_DSP_USE_SIMD=0
        JUCE_USE_SSE_INTRINSICS=0
    )
endif()
