# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        i2c
        test_main.cpp
        test_i2c_writer.cpp
        test_i2c_poll_task.cpp
        test_i2c_poller.cpp
        test_i2c_task.cpp
        test_i2c_poll_impl.cpp
)

target_include_directories(i2c PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(i2c
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(i2c
        PUBLIC
        -Wall
        -Werror
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy)

target_i2c_simlib(i2c)
target_link_libraries(i2c Catch2::Catch2 common-core common-simulation)


catch_discover_tests(i2c)
add_build_and_test_target(i2c)
