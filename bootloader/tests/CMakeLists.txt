# this CMakeLists.txt file is only used when host-compiling to build tests

find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        bootloader
        test_main.cpp
        test_messages.cpp
        test_message_handler.cpp
        test_util.cpp
)

target_include_directories(
        bootloader
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
)

set_target_properties(bootloader
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(bootloader
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
target_link_libraries(bootloader Catch2::Catch2 bootloader-core)

add_revision(TARGET bootloader REVISION a1)

catch_discover_tests(bootloader)
add_build_and_test_target(bootloader)

add_coverage(bootloader)
