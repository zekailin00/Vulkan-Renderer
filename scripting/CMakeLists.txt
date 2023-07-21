

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

file(GLOB ASSET_MANAGER_HEADERS "../asset_manager/*.h")
file(GLOB ASSET_MANAGER_SOURCE "../asset_manager/*.cpp")

add_library(scripting_subsystem

    script.cpp
    script_context.cpp
    scripting_subsystem.cpp
    script_component.cpp
    environment/script_math.cpp
    environment/environment.cpp
    environment/entity_callbacks.cpp
    environment/scene_callbacks.cpp
    environment/input_callbacks.cpp
    environment/component_callbacks.cpp
    environment/asset_manager_callbacks.cpp
    environment/physics/collision_shapes.cpp

    ${ASSET_MANAGER_SOURCE}
    ${ASSET_MANAGER_SOURCE}
)

target_link_libraries(scripting_subsystem PUBLIC
    v8_engine
    engine_core
    renderer
    physics_subsystem
)

target_include_directories(scripting_subsystem PUBLIC
    .
    ../asset_manager
)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)