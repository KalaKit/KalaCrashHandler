cmake_minimum_required(VERSION 3.30.3)

# Ensure install path is set correctly
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out/install")
endif()

if (WIN32)
    # Enable hot reload for MSVC
    if (POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
    # Explicitly set the MSVC toolset for Visual Studio
    if (MSVC)
        set(CMAKE_GENERATOR_TOOLSET "v143")
    endif()
endif()

# Specify the compiler for Linux builds
if (UNIX)
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_MAJOR_VERSION 1)
set(PROJECT_MINOR_VERSION 0)
set(PROJECT_PATCH_VERSION 0)
set(PROJECT_VERSION "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_VERSION}")

project("CrashHandler" VERSION ${PROJECT_VERSION})

# Set runtime library type for MSVC
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Paths to frequently used directories
get_filename_component(PARENT_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)
set(PARENT_DIR_PATH "${PARENT_DIR}")

# Physics files
set(CRASH_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(CRASH_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# Add source files
file(GLOB_RECURSE CRASH_SRC_FILES CONFIGURE_DEPENDS
    ${CRASH_SRC_DIR}/*.cpp
)

# Build as a shared library (DLL)
add_library(CrashHandler SHARED ${CRASH_SRC_FILES})
set_target_properties(CrashHandler PROPERTIES OUTPUT_NAME "CrashHandler")
target_compile_features(CrashHandler PRIVATE cxx_std_20)

# Append 'D' suffix to debug builds for both Windows and Linux
set_target_properties(CrashHandler PROPERTIES
    OUTPUT_NAME "CrashHandler$<$<CONFIG:Debug>:D>"
)

# Apply all directories to target
target_include_directories(CrashHandler PRIVATE
	# Library files
    ${CRASH_INCLUDE_DIR}
)

if (WIN32)
    target_link_libraries(CrashHandler PRIVATE)
else()
    find_package(X11 QUIET)
    if (X11_FOUND)
        target_link_libraries(CrashHandler PRIVATE ${X11_LIBRARIES})
    endif()
endif()

# Define preprocessor directives
target_compile_definitions(CrashHandler PRIVATE CRASHHANDLER_DLL_EXPORT)

# Install the DLL and its import library (CrashHandler.dll + CrashHandler.lib)
install(TARGETS CrashHandler
    RUNTIME DESTINATION bin  # .dll (Windows) and .so (Linux)
    LIBRARY DESTINATION lib  # .lib (Windows import lib) or .so (Linux)
    ARCHIVE DESTINATION lib  # .lib (Linux static lib)
)

# Install header files
install(DIRECTORY ${CRASH_INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)