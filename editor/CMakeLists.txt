file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")


add_executable(editor
    ${LOCAL_HEADERS}
    ${LOCAL_SOURCE}
)

target_link_libraries(editor engine)

add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:editor>/resources
)