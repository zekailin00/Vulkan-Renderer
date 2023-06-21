add_executable(scene1 scene1.cpp)
target_link_libraries(scene1 engine)

add_custom_command(TARGET scene1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:scene1>/resources
)

add_executable(scene2 scene2.cpp)
target_link_libraries(scene2 engine)

add_custom_command(TARGET scene2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:scene2>/resources
)

add_executable(scene3 scene3.cpp)
target_link_libraries(scene3 engine)

add_custom_command(TARGET scene3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:scene3>/resources
)