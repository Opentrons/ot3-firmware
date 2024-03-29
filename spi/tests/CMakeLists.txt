# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        spi
        test_main.cpp
        test_spi.cpp
        test_task.cpp
        test_writer.cpp)

target_include_directories(spi PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(spi
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(spi
        PUBLIC

        -Wall
        -Wextra
        -Wno-missing-field-initializers
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

add_revision(TARGET spi REVISION a1)

target_spi_simlib(spi)
target_link_libraries(spi PUBLIC Catch2::Catch2 common-core)


catch_discover_tests(spi)
add_build_and_test_target(spi)

add_coverage(spi)
