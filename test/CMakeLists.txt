add_executable(testAssetManager1 test_asset_manager1.cpp)

target_link_libraries(testAssetManager1 engine)
add_test(NAME testAssetManager1 COMMAND testAssetManager1)

add_custom_command(TARGET testAssetManager1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:testAssetManager1>/resources
)