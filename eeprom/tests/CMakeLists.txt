# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        eeprom
        test_main.cpp
        test_eeprom_task.cpp
        test_hardware_iface.cpp
)

target_include_directories(eeprom PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(eeprom
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(eeprom
        PUBLIC
        -Wall
        -Werror
        -Wextra
        -Wno-missing-field-initializers
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy)

target_link_libraries(eeprom Catch2::Catch2)
target_i2c_simlib(eeprom)

catch_discover_tests(eeprom)
add_build_and_test_target(eeprom)
