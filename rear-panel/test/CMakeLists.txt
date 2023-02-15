# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        rear_panel
        test_main.cpp
        test_messages.cpp
)

target_include_directories(rear_panel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set_target_properties(rear_panel
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(rear_panel
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

add_revision(TARGET rear_panel REVISION b1)

target_link_libraries(rear_panel PUBLIC Catch2::Catch2)

catch_discover_tests(rear_panel)
add_build_and_test_target(rear_panel)

add_coverage(rear_panel)
