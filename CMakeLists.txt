cmake_minimum_required(VERSION 3.22)
project(TRUCK_STATION VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Si existe la carpeta ./JUCE se usa esa; si no, se descarga JUCE automaticamente.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.12
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(TRUCK_STATION
    COMPANY_NAME "Truck Station Audio"
    BUNDLE_ID "audio.truckstation.instrument"
    PLUGIN_MANUFACTURER_CODE TrSt
    PLUGIN_CODE Trk1
    FORMATS VST3 Standalone
    VST3_CATEGORIES Instrument Synth
    PRODUCT_NAME "TRUCK STATION"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# IMPORTANTE: sin esto no existe <JuceHeader.h> y el proyecto NO compila.
juce_generate_juce_header(TRUCK_STATION)

target_sources(TRUCK_STATION PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_features(TRUCK_STATION PRIVATE cxx_std_17)

target_link_libraries(TRUCK_STATION PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(TRUCK_STATION PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
