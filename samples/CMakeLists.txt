
add_executable(test
    test.cpp
)

target_link_libraries(test renderer)

# Build settings
set_property(TARGET test PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET test POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:test>/resources)