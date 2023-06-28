

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

add_library(scripting_subsystem

    script.cpp
    script_context.cpp
    scripting_subsystem.cpp
    environment/environment.cpp
    environment/entity_callbacks.cpp
)

target_link_libraries(scripting_subsystem PUBLIC
    v8_engine
    engine_core
)

target_include_directories(scripting_subsystem PUBLIC
    .
)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)