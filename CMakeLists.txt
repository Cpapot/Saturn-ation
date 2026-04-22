cmake_minimum_required(VERSION 3.15)

project(SATURN_ATION VERSION 0.0.1)

# 1. Framework JUCE
add_subdirectory("$ENV{USERPROFILE}/Documents/JUCE" JUCE) 

# 2. Plugin Definition
juce_add_plugin(Saturnation
    COMPANY_NAME "MyCompany"
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
)

# 4. Libraries
target_link_libraries(Saturnation PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_include_directories(Saturnation PRIVATE JuceLibraryCode)

# --- AUTO-COPY OF BINARY FILES ---
add_custom_command(TARGET Saturnation_Standalone POST_BUILD
    # VST3 folder copy only if modified
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/Saturnation_artefacts/Release/VST3/Saturn-ation.vst3" 
        "${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.vst3"
    
    # Copy the Standalone .exe only if modified
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/Saturnation_artefacts/Release/Standalone/Saturn-ation.exe" 
        "${CMAKE_CURRENT_SOURCE_DIR}/Saturn-ation.exe"
    
    COMMENT "Verification and copy of modified binaries..."
)