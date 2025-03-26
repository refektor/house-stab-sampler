cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(HouseStabSamples VERSION 0.0.1)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We're going to use CPM as our package manager to bring in JUCE
# Check to see if we have CPM installed already.  Bring it in if we don't.
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# Bring in JUCE locally
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
)

# Make sure you include any new source files here
set(SourceFiles
        source/PluginEditor.cpp
        source/PluginEditor.h
        source/PluginProcessor.cpp
        source/PluginProcessor.h
)

# Change these to your own preferences
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME neighbours
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Neighbours
        PLUGIN_CODE Reg0
        FORMATS VST3 AU Standalone
        NEEDS_WEBVIEW2 TRUE  
        PRODUCT_NAME "HouseStabSamples" # This can't have whitespaces or the create_pkg.sh script will fail
)

# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
)

# Collect all .wav files in the presets folder
file(GLOB WAV_FILES "${CMAKE_CURRENT_SOURCE_DIR}/presets/*.wav")
# Collect all resource files in the ui/public directory and subdirectories
file(GLOB_RECURSE RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/public/*")
# Add the collected .wav files to juce_add_binary_data
juce_add_binary_data(BinaryData
    SOURCES
    ${WAV_FILES}
    ${RESOURCE_FILES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE BinaryData)


# # Add the collected resource files to juce_add_binary_data
# juce_add_binary_data(UiResourcesBinaryData
#     HEADER_NAME "UiResources.h"
#     SOURCES
#     ${RESOURCE_FILES}
# )
# target_link_libraries(${PROJECT_NAME} PRIVATE UiResourcesBinaryData)

# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/juce-src/modules/juce_gui_extra/native/javascript/" 
        DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/public/js/juce/")

# Rename juce's index.js to juce_index.js to avoid conflicts with our own index.js building the juce binary data
file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/public/js/juce/index.js" "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/public/js/juce/juce_index.js")        

set_target_properties(HouseStabSamples PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts"
)