cmake_policy(SET CMP0091 NEW)
cmake_minimum_required(VERSION 3.23.0)

project(SemaLink_Engine)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)
set(CMAKE_CXX_STANDARD 17)

# The following two are required for linking v8 
# to the rest of the application on Windows OS
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)

# PhysX needs this defintion.
# Visual Studio on Windows adds this automatically
# but it needs manual defintion on Mac
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

file(GLOB PLT_HEADERS "platform/*.h")
file(GLOB PLT_SOURCE "platform/*.cpp")

# V8 engine
add_subdirectory("${CMAKE_SOURCE_DIR}/external/v8-library")

# Physx
add_subdirectory("${CMAKE_SOURCE_DIR}/external/physx-library")

# Vulkan
if (APPLE)
# 1.3.239.0 MBP
# 1.3.224.1 MBA
    message("-- Apple MacOS platform. Locate Vulkan manually.")
    set(Vulkan_PATH "/Users/zekailin00/VulkanSDK/1.3.239.0")
    set(Vulkan_INCLUDE_DIRS "${Vulkan_PATH}/macOS/include")
    set(Vulkan_LOADER "${Vulkan_PATH}/macOS/lib/libvulkan.dylib")
    #set(Vulkan_DRIVER "${Vulksn_PATH}/MoltenVK/dylib/macOS/libMoltenVK.dylib")
    set(Vulkan_LIBRARY ${Vulkan_LOADER} ${Vulkan_DRIVER})
else()
    message("-- Windows platform. Locate Vulkan automatically.")
    find_package(Vulkan REQUIRED)
    message( "-- Vulkan found: ${Vulkan_FOUND}")
    message( "-- Vulkan include directories: ${Vulkan_INCLUDE_DIRS}")
    message( "-- Vulkan libraries: ${Vulkan_LIBRARY}")
endif()

# openxr
option(DYNAMIC_LOADER "Build the loader as a .dll library" OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/OpenXR-SDK")

# tracy
option(TRACY_ENABLE "" ON)
add_subdirectory ("${CMAKE_SOURCE_DIR}/external/tracy")

# GLFW
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/external/glfw") # Set this to point to an up-to-date GLFW repo
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR})

# Glm
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glm")

# Dear ImGui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

# tiny obj loader
set(TINY_OBJ_DIR "${CMAKE_SOURCE_DIR}/external/tiny_obj_loader/tiny-obj-loader-submodule/include")

# json cpp
option(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/jsoncpp")

# utility
add_subdirectory("${CMAKE_SOURCE_DIR}/utility")

# core
add_subdirectory("${CMAKE_SOURCE_DIR}/core")

# Physics subsystem
add_subdirectory("${CMAKE_SOURCE_DIR}/physics")

# Renderer
add_subdirectory("${CMAKE_SOURCE_DIR}/renderer")

# platform library
add_library(platform
    ${PLT_HEADERS}
    ${PLT_SOURCE}
)

target_include_directories(platform PUBLIC
    platform
)

target_link_libraries(platform
    renderer
    openxr_loader
    glfw
)

# Scripting subsystem
add_subdirectory("${CMAKE_SOURCE_DIR}/scripting")

# Target settings
add_library(engine
    application.cpp
)

# Target settings
target_include_directories(engine PUBLIC
    .
)

# libraries
target_link_libraries(engine
    scripting_subsystem
    physics_subsystem
    renderer
    platform
    engine_core
)


message("----------- CMake and Compiler Configurations -----------")
message(" * Compiler path:\n${CMAKE_CXX_COMPILER}")
message(" * Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(" * Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message(" * Operating System: ${CMAKE_SYSTEM_NAME}")
message(" * Source Directory: ${CMAKE_SOURCE_DIR}")
message(" * Binary Directory: ${CMAKE_BINARY_DIR}")
get_target_property(TARGET_CXX_STANDARD engine CXX_STANDARD)
message(" * Target C++ Standard: ${TARGET_CXX_STANDARD}")
get_target_property(TARGET_LIBRARIES engine LINK_LIBRARIES)
message(" * Target Linked Libraries: ${TARGET_LIBRARIES}")
get_target_property(TARGET_INCLUDE_DIRS engine INCLUDE_DIRECTORIES)
string(REPLACE ";" "\n" TARGET_INCLUDE_DIRS "${TARGET_INCLUDE_DIRS}")
message(" * Target Include Directories:\n${TARGET_INCLUDE_DIRS}")
get_target_property(TARGET_SOURCES engine SOURCES)
string(REPLACE ";" "\n" TARGET_SOURCES "${TARGET_SOURCES}")
message(" * Target Sources:\n${TARGET_SOURCES}")
message("--------------------------------------------------------")


add_subdirectory("${CMAKE_SOURCE_DIR}/editor")
add_subdirectory("${CMAKE_SOURCE_DIR}/samples")
add_subdirectory("${CMAKE_SOURCE_DIR}/test")