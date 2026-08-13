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
# Three faces per the DPD brand kit: DPD Display (wordmark/display),
# Inter (body/controls), JetBrains Mono (metadata/readouts).
# ----------------------------------------------------------------------------
juce_add_binary_data(CabRotBinaryData
    HEADER_NAME BinaryData.h
    NAMESPACE   BinaryData
    SOURCES
        Resources/fonts/DPDDisplay-Regular.ttf
        Resources/fonts/Inter-Regular.ttf
        Resources/fonts/Inter-Medium.ttf
        Resources/fonts/JetBrainsMono-Regular.ttf
)
set_target_properties(CabRotBinaryData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    FOLDER                    "Generated")

set(CABROT_THEME_SOURCES
    Source/Theme/Palette.h
    Source/Theme/Fonts.h
    Source/Theme/Fonts.cpp
    Source/Theme/SpectreLookAndFeel.h
    Source/Theme/SpectreLookAndFeel.cpp
)

set(CABROT_UI_ATOMS
    Source/UI/atoms/DpdMark.h
    Source/UI/atoms/DpdMark.cpp
    Source/UI/atoms/GhostToggle.h
    Source/UI/atoms/GhostToggle.cpp
    Source/UI/atoms/LivePill.h
    Source/UI/atoms/LivePill.cpp
    Source/UI/atoms/MeterPill.h
    Source/UI/atoms/MeterPill.cpp
    Source/UI/atoms/ModeButton.h
    Source/UI/atoms/ModeButton.cpp
    Source/UI/atoms/SpectreKnob.h
    Source/UI/atoms/SpectreKnob.cpp
)

set(CABROT_UI_REGIONS
    Source/UI/AmpProfileGrid.h
    Source/UI/AmpProfileGrid.cpp
    Source/UI/FizzReadout.h
    Source/UI/FizzReadout.cpp
    Source/UI/FooterBar.h
    Source/UI/FooterBar.cpp
    Source/UI/HeaderBar.h
    Source/UI/HeaderBar.cpp
    Source/UI/KnobRow.h
    Source/UI/KnobRow.cpp
    Source/UI/WaspMeter.h
    Source/UI/WaspMeter.cpp
)

set(CABROT_DSP_SOURCES
    Source/DSP/Tuning.h
    Source/DSP/ModeConfig.h
    Source/DSP/InputTrim.h
    Source/DSP/InputTrim.cpp
    Source/DSP/BandSplitter.h
    Source/DSP/BandSplitter.cpp
    Source/DSP/TransientDetector.h
    Source/DSP/TransientDetector.cpp
    Source/DSP/DynamicReducer.h
    Source/DSP/DynamicReducer.cpp
    Source/DSP/ReapMixer.h
    Source/DSP/ReapMixer.cpp
)

set(CABROT_PRESET_SOURCES
    Source/Presets/FactoryPresets.h
    Source/Presets/PresetManager.h
    Source/Presets/PresetManager.cpp
)

set(CABROT_SHARED_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    ${CABROT_PRESET_SOURCES}
    ${CABROT_THEME_SOURCES}
    ${CABROT_UI_ATOMS}
    ${CABROT_UI_REGIONS}
    ${CABROT_DSP_SOURCES}
)

target_sources(CabRot PRIVATE ${CABROT_SHARED_SOURCES})

target_include_directories(CabRot PRIVATE Source)

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
        juce::juce_dsp
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
             FILES ${CABROT_SHARED_SOURCES})

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
            ${CABROT_SHARED_SOURCES})

    target_include_directories(CabRot_PassthroughTest PRIVATE Source)

    target_compile_definitions(CabRot_PassthroughTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_STANDALONE_APPLICATION=1)

    target_link_libraries(CabRot_PassthroughTest
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
            CabRotBinaryData
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    if(MSVC)
        target_compile_options(CabRot_PassthroughTest PRIVATE /wd4458)
    endif()

    set_target_properties(CabRot_PassthroughTest PROPERTIES FOLDER tests)

    add_test(NAME passthrough COMMAND CabRot_PassthroughTest)
    set_tests_properties(passthrough PROPERTIES TIMEOUT 30)

    # Phase 4 gate. Measures the band split, the transient protection, and
    # the sample-rate matrix rather than eyeballing them in a DAW.
    juce_add_console_app(CabRot_DspTest
        PRODUCT_NAME    "CabRot DSP Test"
        COMPANY_NAME    "Dead Pixel Harmonix"
        BUNDLE_ID       "com.deadpixelharmonix.cabrot.dsptest")

    target_sources(CabRot_DspTest
        PRIVATE
            tests/dsp_test.cpp
            ${CABROT_SHARED_SOURCES})

    target_include_directories(CabRot_DspTest PRIVATE Source)

    target_compile_definitions(CabRot_DspTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_MODAL_LOOPS_PERMITTED=1
            JUCE_STANDALONE_APPLICATION=1)

    target_link_libraries(CabRot_DspTest
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
            CabRotBinaryData
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    if(MSVC)
        target_compile_options(CabRot_DspTest PRIVATE /wd4458)
    endif()

    set_target_properties(CabRot_DspTest PROPERTIES FOLDER tests)

    add_test(NAME dsp COMMAND CabRot_DspTest)
    set_tests_properties(dsp PROPERTIES TIMEOUT 180)
endif()

# ----------------------------------------------------------------------------
# Phase 1 visual-diff harness: a standalone GUI app that renders ThemeTest.
# Not a plugin format; not shipped to users. Used by tools/visual-diff.ps1
# with -ThemeTest to capture the theme reference frame.
# ----------------------------------------------------------------------------
option(CABROT_BUILD_THEME_TEST "Build the Phase 1 ThemeTest GUI app target" ON)

if(CABROT_BUILD_THEME_TEST)
    juce_add_gui_app(CabRot_ThemeTest
        PRODUCT_NAME    "Cab Rot Theme Test"
        COMPANY_NAME    "Dead Pixel Harmonix"
        BUNDLE_ID       "com.deadpixelharmonix.cabrot.themetest"
        VERSION         ${PROJECT_VERSION})

    target_sources(CabRot_ThemeTest
        PRIVATE
            Source/UI/ThemeTestApp.cpp
            Source/UI/ThemeTest.cpp
            Source/UI/ThemeTest.h
            Source/Theme/Palette.h
            Source/Theme/Fonts.h
            Source/Theme/Fonts.cpp
            Source/Theme/SpectreLookAndFeel.h
            Source/Theme/SpectreLookAndFeel.cpp)

    target_include_directories(CabRot_ThemeTest PRIVATE Source)

    target_compile_definitions(CabRot_ThemeTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0)

    target_link_libraries(CabRot_ThemeTest
        PRIVATE
            juce::juce_gui_basics
            juce::juce_graphics
            CabRotBinaryData
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    if(MSVC)
        target_compile_options(CabRot_ThemeTest PRIVATE /wd4458)
    endif()

    set_target_properties(CabRot_ThemeTest PROPERTIES FOLDER "Phase1Tools")
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
