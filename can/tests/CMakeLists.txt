# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        can
        test_main.cpp
        test_parse.cpp
        test_messages.cpp
        test_can_bus.cpp
        test_can_message_buffer.cpp
        test_dispatch.cpp
        test_arbitration_id.cpp
        test_bit_timings.cpp
)

target_include_directories(can PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set_target_properties(can
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(can
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

add_revision(TARGET can REVISION a1)

target_link_libraries(can PUBLIC can-core version-lib Catch2::Catch2)

catch_discover_tests(can)
add_build_and_test_target(can)

add_coverage(can)
