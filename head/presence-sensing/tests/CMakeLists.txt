# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(presence-sensing
        test_main.cpp
        test_presence_sensing_driver.cpp
        test_presence_sensing_queue.cpp
)

target_presence_sensing(presence-sensing)

set_target_properties(presence-sensing
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(presence-sensing
        PUBLIC
        -Wall
        -Werror
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy)
target_link_libraries(presence-sensing PUBLIC Catch2::Catch2)

catch_discover_tests(presence-sensing)
add_build_and_test_target(presence-sensing)
