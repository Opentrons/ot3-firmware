# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        sensors
        test_main.cpp
        test_capacitive_sensor.cpp
        test_environment_sensor.cpp
        test_pressure_sensor.cpp
        test_pressure_driver.cpp
)

target_include_directories(sensors PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(sensors
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(sensors
        PUBLIC
        -Wall
        -Werror
        -Weffc++
        -Wextra
        -Wno-missing-field-initializers
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy)

target_link_libraries(sensors Catch2::Catch2 common-simulation common-core motor-utils)
target_i2c_simlib(sensors)

catch_discover_tests(sensors)
add_build_and_test_target(sensors)
