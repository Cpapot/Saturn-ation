cmake_minimum_required(VERSION 3.15)

project(SATURN_ATION VERSION 0.0.1)

# 1. Framework JUCE
# Adjust the path to your JUCE directory as needed.
add_subdirectory("$ENV{USERPROFILE}/Documents/JUCE" JUCE)

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
set(VST3_SYSTEM_DIR "C:/Program Files/Common Files/VST3")
set(VST3_DEBUG_DIR "${CMAKE_CURRENT_BINARY_DIR}/debug_vst3")
set(STANDALONE_DEBUG_DIR "${CMAKE_CURRENT_BINARY_DIR}/debug_standalone")

# Copy the VST3 package after build.
# Debug builds stay inside the build tree; non-Debug builds deploy to the system VST3 folder.
add_custom_command(TARGET Saturnation_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<IF:$<CONFIG:Debug>,${VST3_DEBUG_DIR},${VST3_SYSTEM_DIR}>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/Saturnation_artefacts/$<CONFIG>/VST3/Saturn-ation.vst3"
        "$<IF:$<CONFIG:Debug>,${VST3_DEBUG_DIR}/Saturn-ation.vst3,${VST3_SYSTEM_DIR}/Saturn-ation.vst3>"
    COMMENT "Deploying Saturn-ation.vst3 for $<CONFIG>"
)

# If Standalone target exists, copy its Debug executable to a local debug folder for easy testing
if (TARGET Saturnation_Standalone)
    add_custom_command(TARGET Saturnation_Standalone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${STANDALONE_DEBUG_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/Saturnation_artefacts/$<CONFIG>/Standalone/Saturn-ation.exe"
            "$<IF:$<CONFIG:Debug>,${STANDALONE_DEBUG_DIR}/Saturn-ation.exe,${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.exe>"
        COMMENT "Copying standalone executable for $<CONFIG>"
    )
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