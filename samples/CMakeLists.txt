
add_executable(test
    test.cpp
)

target_link_libraries(test renderer)

# Build settings
set_property(TARGET test PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET test POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:test>/resources)


add_executable(test_scene scene.cpp)
target_link_libraries(test_scene renderer)
set_property(TARGET test_scene PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET test_scene POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:test_scene>/resources)



                   