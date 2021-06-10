# CMake 3.19 is the first to include presets
# (https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
cmake_minimum_required(VERSION 3.19)

project("OT3Firmware"
  LANGUAGES CXX C ASM
  DESCRIPTION "Firmware for OT-3 System")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
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

add_subdirectory(pipettes)

find_package(Clang)

file(GLOB_RECURSE SOURCES_FOR_FORMAT ./*.cpp ./*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
# add_custom_target(
#   format
#   ALL
#   COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${SOURCES_FOR_FORMAT}
#   )


# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
  format-ci
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${SOURCES_FOR_FORMAT}
)

find_package(flatbuffers)

# The flatbuffer CMakeLists.txt file will use Apple's toolchain if
# CMAKE_TOOLCHAIN_FILE is not defined. This circumvents that.
if (CMAKE_COMPILER_IS_GNUCXX)
  set(APPLE FALSE)
endif()

# Add FlatBuffers directly to our build. This defines the `flatbuffers` target.
add_subdirectory(${FlatBuffers_SOURCE_DIR}
                 ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
                 EXCLUDE_FROM_ALL)

add_subdirectory(messages)