# CMake 3.19 is the first to include presets
# (https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
cmake_minimum_required(VERSION 3.19)

project("OT3Firmware"
  LANGUAGES CXX C ASM
  DESCRIPTION "Firmware for OT-3 System")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(ROOT_BOARD_FILE_PATH "${PROJECT_SOURCE_DIR}/include/firmware/" ${ROOT_BOARD_FILE_PATH})
if (NOT ${CMAKE_CROSSCOMPILING})
  enable_testing()
  add_custom_target(build-and-test)
endif()

# Set a default build type if none was specified
# From https://blog.kitware.com/cmake-and-the-default-build-type/

# MinSizeRel is the default release type since the binary size is quite important.
set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build." FORCE)
# Set the possible values of build type for cmake-gui
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "MinSizeRel" "RelWithDebInfo")
endif()
message("Build type is ${CMAKE_BUILD_TYPE}")

if (${CMAKE_CROSSCOMPILING})
  find_package(CrossGCC)
  find_package(OpenOCD)
endif()

add_subdirectory(common)
add_subdirectory(pipettes)
add_subdirectory(gantry)

find_package(Clang)

file(GLOB_RECURSE SOURCES_FOR_FORMAT ./*.cpp ./*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
  format
  ALL
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${SOURCES_FOR_FORMAT}
  )

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
  format-ci
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${SOURCES_FOR_FORMAT}
)
