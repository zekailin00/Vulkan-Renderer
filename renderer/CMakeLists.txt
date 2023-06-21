cmake_minimum_required(VERSION 3.1.0)

# Build settings

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")
file(GLOB PRIM_HEADERS "vk_primitives/*.h")
file(GLOB PRIM_SOURCE "vk_primitives/*.cpp")
file(GLOB LOADER_HEADERS "loaders/*.h")
file(GLOB LOADER_SOURCE "loaders/*.cpp")
file(GLOB COMPONENT_HEADERS "components/*.h")
file(GLOB COMPONENT_SOURCE "components/*.cpp")

# Dear ImGui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
file(GLOB IMGUI_SOURCE "${IMGUI_DIR}/*.cpp")

# stb image
set(STB_IMAGE "${CMAKE_SOURCE_DIR}/external/stb/include")

add_library(renderer    
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
    ${PRIM_HEADERS}
    ${PRIM_SOURCE}
    ${LOADER_HEADERS}
    ${LOADER_SOURCE}
    ${COMPONENT_HEADERS}
    ${COMPONENT_SOURCE}

    ${IMGUI_SOURCE}
)

# Target settings
target_include_directories(renderer PUBLIC 
    ${Vulkan_INCLUDE_DIRS}

    ${IMGUI_DIR}
    ${STB_IMAGE}
    ${TINY_OBJ_DIR}

    components
    interface
    loaders
    .
)

target_link_libraries(renderer PUBLIC
    ${Vulkan_LIBRARY}
    engine_core
)