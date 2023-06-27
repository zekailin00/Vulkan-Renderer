


add_executable(test_script_compile
    test_script_compile.cpp
)

target_link_libraries(test_script_compile PUBLIC
    scripting_subsystem
)

target_include_directories(test_script_compile PUBLIC
    .
)