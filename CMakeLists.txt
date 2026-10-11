cmake_minimum_required(VERSION 3.22)

project(SimonsExpensifier VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE 8.0.4 lives in ./JUCE (git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git JUCE)
add_subdirectory(JUCE)

# AU only exists on macOS. AAX builds with the SDK bundled in JUCE 8, but
# release Pro Tools only loads AAX plugins that are PACE-signed (see README).
set(PLUGIN_FORMATS VST3 AAX Standalone)
if(APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

juce_add_plugin(SimonsExpensifier
    COMPANY_NAME "Rich Audio"
    BUNDLE_ID "com.richaudio.simonsexpensifier"
    PLUGIN_MANUFACTURER_CODE Rich
    PLUGIN_CODE Dlr3
    PRODUCT_NAME "Simons Expensifier"
    PLUGIN_NAME "Simon's Expensifier"
    DESCRIPTION "Makes your audio expensive. Does nothing."
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx
    AU_MAIN_TYPE kAudioUnitType_Effect
    AAX_CATEGORY AAX_ePlugInCategory_None
    FORMATS ${PLUGIN_FORMATS})

juce_add_binary_data(SimonsExpensifierAssets
    SOURCES
        Resources/background.png
        Resources/glow_dim.png
        Resources/glow_crisp.png
        Resources/glow_halo.png)

target_sources(SimonsExpensifier
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(SimonsExpensifier
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(SimonsExpensifier
    PRIVATE
        SimonsExpensifierAssets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
