cmake_minimum_required(VERSION 3.1.0)

project(vulkan_renderer)

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

# Vulkan
if (APPLE)
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

# Target settings
add_executable(app
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
)

target_include_directories(app PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(app ${Vulkan_LIBRARY})