# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(motor-control
        test_main.cpp
        test_motor_interrupt_queue.cpp
        test_motor_pulse.cpp
        test_linear_motion_config.cpp
        test_move_group.cpp
        test_fixed_point_utils.cpp
        test_motor_driver.cpp
        test_limit_switch.cpp
        test_move_status_handling.cpp
        test_sync_handling.cpp
        test_brushed_motor_interrupt_handler.cpp
        test_motor_flags.cpp
        test_motor_position_tracker.cpp
        test_stall_check.cpp
        )

target_ot_motor_control(motor-control)

set_target_properties(motor-control
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(motor-control
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
target_link_libraries(motor-control PUBLIC Catch2::Catch2 motor-utils)

catch_discover_tests(motor-control)
add_build_and_test_target(motor-control)

add_coverage(motor-control)