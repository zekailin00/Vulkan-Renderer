file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

add_library(engine_core
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
)

target_include_directories(engine_core PUBLIC
    .
)

target_link_libraries(engine_core PUBLIC
    utility
)

add_subdirectory(tests)