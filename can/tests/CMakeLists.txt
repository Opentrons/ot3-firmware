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
  -Weffc++
  -Wreorder
  -Wsign-promo
  -Wextra-semi
  -Wctor-dtor-privacy
  -fno-rtti)
target_link_libraries(can can-core Catch2::Catch2)

catch_discover_tests(can)
add_build_and_test_target(can)
