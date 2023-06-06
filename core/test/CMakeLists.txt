
message("-- Engine core tests found.")
add_executable(basicTest basic_test.cpp)

target_link_libraries(basicTest engine_core)
add_test(NAME basicTest COMMAND basicTest)