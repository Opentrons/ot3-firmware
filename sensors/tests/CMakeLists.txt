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
        test_environment_driver.cpp
        test_pressure_sensor.cpp
        test_pressure_driver.cpp
        test_capacitive_sensor_utils.cpp
        test_sensor_hardware.cpp
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
        -Wextra
        -Wno-missing-field-initializers
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_link_libraries(sensors PUBLIC Catch2::Catch2 Boost::boost Boost::program_options Boost::date_time common-core motor-utils common-simulation)

target_i2c_simlib(sensors)

catch_discover_tests(sensors)
add_build_and_test_target(sensors)

add_coverage(sensors)
