# this CMakeLists.txt file is only used when host-compiling to build tests

find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        hepa-uv
        test_main.cpp
        test_leds.cpp
)

target_include_directories(hepa-uv PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(hepa-uv
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(hepa-uv
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
add_revision(TARGET hepa-uv REVISION b1)

target_link_libraries(hepa-uv Catch2::Catch2)

catch_discover_tests(hepa-uv)
add_build_and_test_target(hepa-uv)

add_coverage(hepa-uv)
