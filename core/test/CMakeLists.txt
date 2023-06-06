
add_executable(core_test
    main.cpp
)

target_link_libraries(core_test PUBLIC
    engine_core
)