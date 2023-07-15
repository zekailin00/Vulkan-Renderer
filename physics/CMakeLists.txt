add_library(physics_subsystem
    physics_system.cpp
    physics_context.cpp
)

target_include_directories(physics_subsystem PUBLIC
    .
)

target_link_libraries(physics_subsystem PUBLIC
    physx
)

add_executable(test_physx_launch
    test_launch/main.cpp
)

target_link_libraries(test_physx_launch PRIVATE
    physics_subsystem
)
