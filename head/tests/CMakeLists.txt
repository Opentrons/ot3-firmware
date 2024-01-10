# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        head
        test_main.cpp
)

add_revision(TARGET head REVISION a1)

target_include_directories(head PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set_target_properties(head
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(head
        PUBLIC
        -Wall
        -Werror
        -Wno-missing-field-initializers
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_link_libraries(head PUBLIC Catch2::Catch2 common-core)

catch_discover_tests(head)
add_build_and_test_target(head)

add_coverage(head)
