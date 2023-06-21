

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

add_executable(scripting_subsystem
scripting_subsystem.h
scripting_subsystem.cpp
environment.h
)

target_link_libraries(scripting_subsystem PUBLIC
    v8_engine
    engine_core
)

target_include_directories(scripting_subsystem PUBLIC
    .
)