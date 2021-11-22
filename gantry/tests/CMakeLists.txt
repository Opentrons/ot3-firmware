# this CMakeLists.txt file is only used when host-compiling to build tests

find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        gantry
        test_main.cpp
        test_linear_motion_config.cpp
)

target_include_directories(gantry PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(gantry
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(gantry
        PUBLIC
        -Wall
        -Werror
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy
        -fno-rtti)
target_link_libraries(gantry Catch2::Catch2 gantry-core-x)

catch_discover_tests(gantry)
add_build_and_test_target(gantry)
