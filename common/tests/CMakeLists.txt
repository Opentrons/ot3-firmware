# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)



add_executable(
        common
        test_main.cpp
        test_bit_utils.cpp
        test_synchronization.cpp
        test_allocator.cpp
        test_debounce.cpp
)

add_revision(TARGET common REVISION "a1")

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set_target_properties(common
  PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE)

target_compile_options(common
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
target_link_libraries(common Catch2::Catch2 common-core)

catch_discover_tests(common)
add_build_and_test_target(common)

add_coverage(common)
