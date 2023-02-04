
# Build settings
file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")

add_library(utility 
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
)

target_link_libraries(utility
    glm::glm
)

# Target settings
target_include_directories(utility PUBLIC 
    .
)

# Build settings
set_property(TARGET utility PROPERTY CXX_STANDARD 17)