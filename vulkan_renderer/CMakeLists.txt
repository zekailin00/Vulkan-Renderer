cmake_minimum_required(VERSION 3.1.0)

# Build settings
file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

# Dear ImGui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
file(GLOB IMGUI_SOURCE "${IMGUI_DIR}/*.cpp")

# stb image
set(STB_IMAGE "${CMAKE_SOURCE_DIR}/external/stb/include")

add_library(VulkanRenderer    
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}

    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_SOURCE}
)

# Target settings
target_include_directories(VulkanRenderer PUBLIC 
    ${Vulkan_INCLUDE_DIRS}
    
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    
    ${STB_IMAGE}
    
    .
)
target_link_libraries(VulkanRenderer 
    ${Vulkan_LIBRARY}
    utility
)


# Build settings
set_property(TARGET VulkanRenderer PROPERTY CXX_STANDARD 17)