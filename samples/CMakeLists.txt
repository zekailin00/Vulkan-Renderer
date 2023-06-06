add_executable(scene1 scene1.cpp)
target_link_libraries(scene1 renderer)
set_property(TARGET scene1 PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET scene1 POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:scene1>/resources)


add_executable(scene2 scene2.cpp)
target_link_libraries(scene2 renderer)
set_property(TARGET scene2 PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET scene2 POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:scene2>/resources)