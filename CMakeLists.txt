cmake_minimum_required(VERSION 3.22)

project(TRUCK_STATION VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(TRUCK_STATION
    COMPANY_NAME "Truck Station Audio"
    BUNDLE_ID "audio.truckstation.soundsystem"
    PLUGIN_MANUFACTURER_CODE TrSt
    PLUGIN_CODE Tr02
    FORMATS VST3 Standalone
    VST3_CATEGORIES Instrument Synth
    PRODUCT_NAME "TRUCK STATION V2"
    DESCRIPTION "Experimental three-oscillator synthesizer"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# IMPORTANTE: sin esto no existe <JuceHeader.h> y el proyecto NO compila.
juce_generate_juce_header(TRUCK_STATION)

target_sources(TRUCK_STATION PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(TRUCK_STATION PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TRUCK_STATION PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
