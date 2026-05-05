cmake_minimum_required(VERSION 3.22)

project(CabRot
    VERSION 0.1.0
    DESCRIPTION "Dynamic harshness controller for high-gain amp-sim guitars"
    LANGUAGES CXX C
)

# ----------------------------------------------------------------------------
# Build settings
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Treat warnings as errors so we never ship a build with hidden noise.
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /MP)
    add_compile_definitions(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
else()
    add_compile_options(-Wall -Wextra -Wshadow -Wpedantic)
endif()

# ----------------------------------------------------------------------------
# JUCE
# ----------------------------------------------------------------------------
add_subdirectory(JUCE EXCLUDE_FROM_ALL)

# ----------------------------------------------------------------------------
# VST3 install location (user-scope, no admin required)
# ----------------------------------------------------------------------------
if(WIN32)
    set(_default_vst3_dir "$ENV{LOCALAPPDATA}/Programs/Common/VST3")
elseif(APPLE)
    set(_default_vst3_dir "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
else()
    set(_default_vst3_dir "$ENV{HOME}/.vst3")
endif()
set(CABROT_VST3_COPY_DIR "${_default_vst3_dir}"
    CACHE PATH "Where the built VST3 is copied after each build (user-scope by default)")

# OFF by default. If Reaper has the VST3 loaded, the post-build copy fails
# because Windows holds the file open. Iterate by pointing Reaper at the
# build artefact dir, or run tools/install-vst3.ps1 once you've closed
# whichever DAW session is holding the file. -DCABROT_COPY_AFTER_BUILD=ON
# forces the old behaviour for first-time installs.
option(CABROT_COPY_AFTER_BUILD "Copy VST3 to CABROT_VST3_COPY_DIR after each build" OFF)

# ----------------------------------------------------------------------------
# Plugin target
# ----------------------------------------------------------------------------
juce_add_plugin(CabRot
    VERSION                     ${PROJECT_VERSION}
    COMPANY_NAME                "Dead Pixel Harmonix"
    COMPANY_WEBSITE             "https://deadpixelharmonix.com"
    COMPANY_EMAIL               "drjr1021@gmail.com"
    BUNDLE_ID                   "com.deadpixelharmonix.cabrot"
    PLUGIN_MANUFACTURER_CODE    Dphx
    PLUGIN_CODE                 Cbrt
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Cab Rot"
    DESCRIPTION                 "${PROJECT_DESCRIPTION}"

    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    VST3_CATEGORIES             Fx Dynamics EQ
    AU_MAIN_TYPE                kAudioUnitType_Effect
    HARDENED_RUNTIME_ENABLED    TRUE

    COPY_PLUGIN_AFTER_BUILD     ${CABROT_COPY_AFTER_BUILD}
    VST3_COPY_DIR               "${CABROT_VST3_COPY_DIR}"
)

juce_generate_juce_header(CabRot)

# ----------------------------------------------------------------------------
# Bundled fonts -> BinaryData (no runtime CDN, no external dependency).
# Phase 1 will add Space Grotesk Black (900) + the rest of the weight ladder.
# ----------------------------------------------------------------------------
juce_add_binary_data(CabRotBinaryData
    HEADER_NAME BinaryData.h
    NAMESPACE   BinaryData
    SOURCES
        Resources/fonts/SpaceGrotesk-Regular.ttf
        Resources/fonts/SpaceGrotesk-Medium.ttf
        Resources/fonts/SpaceGrotesk-Bold.ttf
        Resources/fonts/JetBrainsMono-Regular.ttf
        Resources/fonts/JetBrainsMono-Bold.ttf
)
set_target_properties(CabRotBinaryData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    FOLDER                    "Generated")

target_sources(CabRot
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(CabRot
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=1
)

target_link_libraries(CabRot
    PRIVATE
        juce::juce_audio_utils
        CabRotBinaryData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Per-source override: JUCE auto-generated headers can flake on /W4.
if(MSVC)
    target_compile_options(CabRot PRIVATE /wd4458)
endif()

# Silence C4996 (CRT-secure deprecation) on JUCE's auto-generated VST3 manifest
# helper - the warnings come from the upstream Steinberg VST3 SDK shipped inside
# JUCE 8.0.12 and are not under our control.
if(MSVC AND TARGET CabRot_vst3_helper)
    target_compile_definitions(CabRot_vst3_helper PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(CabRot_vst3_helper PRIVATE /wd4996)
endif()

# Group source files in IDEs.
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
             PREFIX "Source"
             FILES
                 Source/PluginProcessor.cpp
                 Source/PluginProcessor.h
                 Source/PluginEditor.cpp
                 Source/PluginEditor.h)

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
option(CABROT_BUILD_TESTS "Build the Cab Rot test executables" ON)

if(CABROT_BUILD_TESTS)
    enable_testing()

    juce_add_console_app(CabRot_PassthroughTest
        PRODUCT_NAME    "CabRot Passthrough Test"
        COMPANY_NAME    "Dead Pixel Harmonix"
        BUNDLE_ID       "com.deadpixelharmonix.cabrot.passthroughtest")

    target_sources(CabRot_PassthroughTest
        PRIVATE
            tests/passthrough_test.cpp
            Source/PluginProcessor.cpp
            Source/PluginEditor.cpp)

    target_compile_definitions(CabRot_PassthroughTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_STANDALONE_APPLICATION=1)

    target_link_libraries(CabRot_PassthroughTest
        PRIVATE
            juce::juce_audio_utils
            CabRotBinaryData
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    if(MSVC)
        target_compile_options(CabRot_PassthroughTest PRIVATE /wd4458)
    endif()

    set_target_properties(CabRot_PassthroughTest PROPERTIES FOLDER tests)

    add_test(NAME passthrough COMMAND CabRot_PassthroughTest)
    set_tests_properties(passthrough PROPERTIES TIMEOUT 30)
endif()

# ----------------------------------------------------------------------------
# Status banner
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "Cab Rot ${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Generator:      ${CMAKE_GENERATOR}")
message(STATUS "  VST3 copy dir:  ${CABROT_VST3_COPY_DIR}")
message(STATUS "  Copy on build:  ${CABROT_COPY_AFTER_BUILD}")
message(STATUS "")
