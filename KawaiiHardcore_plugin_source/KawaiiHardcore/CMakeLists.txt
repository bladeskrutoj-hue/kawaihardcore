cmake_minimum_required(VERSION 3.22)
project(KawaiiHardcore VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(KawaiiHardcore
    COMPANY_NAME "KawaiiAudio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Kwai
    PLUGIN_CODE Khc1
    FORMATS VST3 Standalone
    PRODUCT_NAME "Kawaii Hardcore")

target_sources(KawaiiHardcore PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(KawaiiHardcore PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(KawaiiHardcore
    PRIVATE juce::juce_audio_utils
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
