# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        pipettes
        test_main.cpp
        test_mount_detection.cpp
        test_gear_move_status_handling.cpp
)

target_include_directories(pipettes PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(pipettes
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(pipettes
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

add_revision(TARGET pipettes REVISION b1)

target_link_libraries(pipettes PUBLIC Catch2::Catch2 common-core motor-utils)
target_i2c_simlib(pipettes)

catch_discover_tests(pipettes)
add_build_and_test_target(pipettes)

add_coverage(pipettes)
