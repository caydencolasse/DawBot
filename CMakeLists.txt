cmake_minimum_required(VERSION 3.22)

# Universal macOS binary (Apple silicon + Intel). Must be set before project().
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures")

project(DawBot VERSION 0.1.0 LANGUAGES C CXX)

option(DAWBOT_COPY_PLUGIN_AFTER_BUILD "Install plugin bundles after building" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# JUCE: uses external/JUCE if you added it as a git submodule, otherwise
# downloads it at configure time.
# ---------------------------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------------------
# 1. DawBot plugin (lives in the DAW): chat coach + MIDI clip drag-out
# ---------------------------------------------------------------------------
juce_add_plugin(DawBot
    COMPANY_NAME              "DawBot"
    COMPANY_WEBSITE           "https://dawbot.ai"
    BUNDLE_ID                 ai.dawbot.plugin
    PLUGIN_MANUFACTURER_CODE  Dwbt
    PLUGIN_CODE               Dbt1
    FORMATS                   AU VST3 Standalone
    PRODUCT_NAME              "DawBot"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES           Fx Tools
    AU_MAIN_TYPE              kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD   ${DAWBOT_COPY_PLUGIN_AFTER_BUILD})

juce_add_binary_data(DawBotAssets HEADER_NAME DawBotAssets.h NAMESPACE DawBotAssets SOURCES
    Resources/logo.png
    Resources/book.svg Resources/command.svg Resources/folder.svg Resources/help.svg
    Resources/history.svg Resources/keys.svg Resources/mic.svg Resources/play.svg
    Resources/settings.svg Resources/wave.svg Resources/info.svg Resources/search.svg
    Resources/InterRegular.ttf Resources/InterSemiBold.ttf Resources/Inter-OFL.txt)

target_sources(DawBot PRIVATE
    Source/Plugin/PluginProcessor.cpp
    Source/Plugin/PluginEditor.cpp
    Source/Plugin/WorkspaceState.cpp
    Source/Plugin/UI/Components.cpp
    Source/Plugin/UI/Composer.cpp
    Source/Plugin/UI/WorkspaceViews.cpp
    Source/Shared/MidiClip.cpp)

target_include_directories(DawBot PRIVATE Source)

target_compile_definitions(DawBot PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DawBot
    PRIVATE juce::juce_audio_utils DawBotAssets
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# 2. DawBot Helper (desktop app): screen capture, AI calls, on-screen overlay
# ---------------------------------------------------------------------------
juce_add_gui_app(DawBotHelper
    PRODUCT_NAME "DawBot Helper"
    COMPANY_NAME "DawBot"
    BUNDLE_ID    ai.dawbot.helper
    VERSION      ${PROJECT_VERSION})

target_sources(DawBotHelper PRIVATE
    Source/Helper/Main.cpp
    Source/Helper/HelperServer.cpp
    Source/Helper/AIClient.cpp
    Source/Helper/Overlay.cpp
    Source/Helper/PluginScanner.cpp
    Source/Helper/Config.cpp
    Source/Helper/ScreenCaptureCommon.cpp)

if(APPLE)
    target_sources(DawBotHelper PRIVATE Source/Helper/ScreenCapture_mac.mm)
    set_source_files_properties(Source/Helper/ScreenCapture_mac.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
    # Weak-linked so the app still launches on macOS 11-13 (capture needs 14+).
    target_link_options(DawBotHelper PRIVATE "SHELL:-weak_framework ScreenCaptureKit")
elseif(WIN32)
    target_sources(DawBotHelper PRIVATE Source/Helper/ScreenCapture_win.cpp)
else()
    target_sources(DawBotHelper PRIVATE Source/Helper/ScreenCapture_stub.cpp)
endif()

target_include_directories(DawBotHelper PRIVATE Source)

target_compile_definitions(DawBotHelper PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DawBotHelper,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DawBotHelper,JUCE_VERSION>")

target_link_libraries(DawBotHelper
    PRIVATE juce::juce_gui_extra
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

option(DAWBOT_BUILD_TESTS "Build native workspace and UI regression tests" OFF)
if(DAWBOT_BUILD_TESTS)
    enable_testing()
    # Permit bounded message-loop pumping only in the optional test build.
    target_compile_definitions(DawBot PUBLIC JUCE_MODAL_LOOPS_PERMITTED=1)
    add_executable(DawBotTests Tests/PluginTests.cpp)
    set_target_properties(DawBotTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/DawBotTests_artefacts/$<CONFIG>")
    target_include_directories(DawBotTests PRIVATE Source)
    target_link_libraries(DawBotTests PRIVATE DawBot juce::juce_audio_utils)
    add_test(NAME DawBotTests COMMAND DawBotTests)
endif()
