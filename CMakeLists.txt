cmake_minimum_required(VERSION 3.15)

project(SATURN_ATION VERSION 0.0.1)

# ============================================================================
# CI/CD Detection
# ============================================================================
# Detect if we're running in CI/CD environment (GitHub Actions, etc.)
if(DEFINED ENV{CI} OR DEFINED ENV{GITHUB_ACTIONS})
    set(IS_CI_BUILD TRUE)
    message(STATUS "CI/CD environment detected - adjusting build configuration")
else()
    set(IS_CI_BUILD FALSE)
endif()

# ============================================================================
# macOS ARM64 (Apple Silicon M1/M2/M3) Configuration
# ============================================================================
# For Release builds, enable macOS ARM64 cross-compilation support
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
    # Set default architecture for macOS to ARM64 (Apple Silicon)
    if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS target architectures")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version for ARM64")
        message(STATUS "Configuring for macOS ARM64 (Apple Silicon M1/M2/M3)")
    endif()
endif()

# ============================================================================
# JUCE Framework Setup
# ============================================================================
# Check for JUCE in multiple locations:
# 1. CI/CD: Use JUCE as git submodule in project root
# 2. Local: Use existing JUCE installation path
set(JUCE_PATH "")

# Try CI/CD submodule location first
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    message(STATUS "Found JUCE submodule at: ${JUCE_PATH}")
# Try local Documents folder (Windows development)
elseif(EXISTS "$ENV{USERPROFILE}/Documents/JUCE/CMakeLists.txt")
    set(JUCE_PATH "$ENV{USERPROFILE}/Documents/JUCE")
    message(STATUS "Found JUCE at: ${JUCE_PATH}")
# Try standard macOS location
elseif(EXISTS "/Users/*/Documents/JUCE/CMakeLists.txt")
    file(GLOB JUCE_PATH "/Users/*/Documents/JUCE")
    message(STATUS "Found JUCE at macOS location: ${JUCE_PATH}")
else()
    message(FATAL_ERROR "JUCE framework not found. Please ensure JUCE is either:\n"
        "  1. Added as a git submodule: git submodule add https://github.com/juce-framework/JUCE.git JUCE\n"
        "  2. Located at: $ENV{USERPROFILE}/Documents/JUCE (Windows)\n"
        "  3. Located at: ~/Documents/JUCE (macOS/Linux)")
endif()

add_subdirectory("${JUCE_PATH}" JUCE)

# 1b. Embedded assets
juce_add_binary_data(SaturnationAssets
    SOURCES
        assets/background.png
        assets/knob_512x512.png
        assets/Outfit_Regular.ttf
)

# 2. Plugin Definition
juce_add_plugin(Saturnation
    COMPANY_NAME "MyCompany"
    PLUGIN_MANUFACTURER_CODE MyCo
    PLUGIN_CODE StnA
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PRODUCT_NAME "Saturn-ation"
    FORMATS VST3 Standalone
)

target_compile_definitions(Saturnation PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ============================================================================
# Release Build Optimization for macOS ARM64
# ============================================================================
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Add ARM64-specific compiler flags for Release builds
    if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # Ensure ARM64 architecture in Release mode
        target_compile_options(Saturnation PRIVATE
            -arch arm64
            -march=armv8-a
            -mcpu=generic
            -O3
            -ffast-math
        )
        target_link_options(Saturnation PRIVATE
            -arch arm64
        )
        message(STATUS "Applied ARM64 optimizations for Release build")
    endif()
endif()

# 3. Sources Files
target_sources(Saturnation PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
	Source/apvts.cpp
	Source/SaturnKnobLookAndFeel.cpp
)

# 4. Libraries
target_link_libraries(Saturnation PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    SaturnationAssets
)

target_include_directories(Saturnation PRIVATE JuceLibraryCode)

# --- DEPLOYMENT & REPOSITORY CLEANUP ---
# Enable this option to copy the built VST3 to the Windows system VST3 folder.
# Usage: cmake -S . -B build -A x64 -DSATURN_COPY_VST3_TO_WINDOWS_SYSTEM=ON
option(SATURN_COPY_VST3_TO_WINDOWS_SYSTEM "Copy built VST3 to Windows VST3 system folder" OFF)

set(VST3_SYSTEM_DIR "C:/Program Files/Common Files/VST3")

set(VST3_DEBUG_DIR "${CMAKE_CURRENT_BINARY_DIR}/debug_vst3")
set(STANDALONE_DEBUG_DIR "${CMAKE_CURRENT_BINARY_DIR}/debug_standalone")

# Copy the VST3 package after build only when the flag is enabled.
# This avoids accidental deployment when building locally or in CI.
if (TARGET Saturnation_VST3 AND WIN32 AND SATURN_COPY_VST3_TO_WINDOWS_SYSTEM AND NOT IS_CI_BUILD)
    add_custom_command(TARGET Saturnation_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${VST3_SYSTEM_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/Saturnation_artefacts/$<CONFIG>/VST3/Saturn-ation.vst3"
            "${VST3_SYSTEM_DIR}/Saturn-ation.vst3"
        COMMENT "Deploying Saturn-ation.vst3 to Windows system VST3 folder for $<CONFIG>"
    )
endif()

# If Standalone target exists, copy its Debug executable to a local debug folder for easy testing
if (TARGET Saturnation_Standalone)
    if (WIN32)
        add_custom_command(TARGET Saturnation_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:Saturnation_Standalone>"
                "${CMAKE_SOURCE_DIR}/Saturn-ation.exe"
            COMMENT "Copying standalone executable for Release (Windows)"
        )
    elseif(APPLE)
        # Option A: copy the .app bundle directory
        add_custom_command(TARGET Saturnation_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_BUNDLE_DIR:Saturnation_Standalone>"
                "${CMAKE_SOURCE_DIR}/Saturn-ation.app"
            COMMENT "Copying standalone app bundle for Release (macOS)"
        )
    endif()
endif()

# Cleanup target to keep repository root clean
add_custom_target(clean_repo_artifacts
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.exe"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.vst3"
    COMMENT "Cleaning repository artifacts at project root"
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.exe"
    "${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.vst3"
    "${VST3_DEBUG_DIR}"
    "${STANDALONE_DEBUG_DIR}"
)