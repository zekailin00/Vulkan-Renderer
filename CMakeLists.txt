cmake_minimum_required(VERSION 3.23.0)

project(SemaLink_Engine)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)
set(CMAKE_CXX_STANDARD 17)
# enable_testing()

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

file(GLOB PLT_HEADERS "platform/*.h")
file(GLOB PLT_SOURCE "platform/*.cpp")

file(GLOB MODEL_LOADERS_HEADERS "model_loaders/*.h")
file(GLOB MODEL_LOADERS_SOURCE "model_loaders/*.cpp")

#v8 engine
add_subdirectory("${CMAKE_SOURCE_DIR}/external/v8-engine")

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


# Target settings
add_library(engine
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
    ${MODEL_LOADERS_HEADERS}
    ${MODEL_LOADERS_SOURCE}
)

# Target settings
target_include_directories(engine PUBLIC
    model_loaders
    .
)

# libraries
target_link_libraries(engine
    renderer
    platform
    engine_core
)

# Build settings
set_property(TARGET engine PROPERTY CXX_STANDARD 17)

add_subdirectory("${CMAKE_SOURCE_DIR}/editor")
add_subdirectory("${CMAKE_SOURCE_DIR}/samples")
add_subdirectory("${CMAKE_SOURCE_DIR}/test")