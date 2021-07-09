# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        pipettes
        test_main.cpp
        test_communication.cpp
)

target_include_directories(pipettes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set_target_properties(pipettes
  PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE)

target_compile_options(pipettes
  PUBLIC
  -Wall
  -Werror
  -Weffc++
  -Wreorder
  -Wsign-promo
  -Wextra-semi
  -Wctor-dtor-privacy
  -fno-rtti)
target_link_libraries(pipettes Catch2::Catch2)

catch_discover_tests(pipettes)
add_build_and_test_target(pipettes)