cmake_minimum_required(VERSION 3.16)
project(TS3PluginPackager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(TSInstaller src/main.cpp)

# Add verification utility
add_executable(verify_ts3plugin src/verify_ts3plugin.cpp)

# Platform-specific settings
if(WIN32)
    # Windows-specific libraries
    target_link_libraries(TSInstaller shlwapi comdlg32)
    
    # Create Windows subsystem version (no console window)
    add_executable(tsinstaller_gui WIN32 src/main.cpp)
    target_link_libraries(tsinstaller_gui shlwapi comdlg32)
    
    # Set properties for GUI version
    set_target_properties(tsinstaller_gui PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Set output directory
set_target_properties(TSInstaller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(verify_ts3plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32 AND TARGET tsinstaller_gui)
    set_target_properties(tsinstaller_gui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
