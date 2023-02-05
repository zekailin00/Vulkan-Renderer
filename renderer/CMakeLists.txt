
# Build settings
file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

add_library(renderer 
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
)

target_link_libraries(renderer
    glm::glm
)

# Target settings
target_include_directories(renderer PUBLIC 
    .
)

# Build settings
set_property(TARGET renderer PROPERTY CXX_STANDARD 17)