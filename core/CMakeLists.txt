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

# Build settings
set_property(TARGET engine_core PROPERTY CXX_STANDARD 17)

add_subdirectory(test)