# CMake 3.19 is the first to include presets
# (https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED ENV{PRESET_IN_USE})
    message(FATAL_ERROR "Use a preset. Use cmake --list-presets to show available presets.")
endif()

project("OT3Firmware"
        LANGUAGES CXX C ASM
        DESCRIPTION "Firmware for OT-3 System")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(ROOT_BOARD_FILE_PATH "${PROJECT_SOURCE_DIR}/include/firmware/" ${ROOT_BOARD_FILE_PATH})
if (NOT ${CMAKE_CROSSCOMPILING})
    enable_testing()
    add_custom_target(build-and-test)
endif ()

# Set a default build type if none was specified
# From https://blog.kitware.com/cmake-and-the-default-build-type/

# MinSizeRel is the default release type since the binary size is quite important.
set(default_build_type "Debug")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "MinSizeRel" "RelWithDebInfo")
endif ()
message("Build type is ${CMAKE_BUILD_TYPE}")


if (${CMAKE_CROSSCOMPILING})
    find_package(CrossGCC)
    find_package(OpenOCD)

    set(STM32L5 0)
    set(STM32G4 0)
    if (${ARM_ARCH_TYPE} STREQUAL "cortex-m33")
        set(STM32L5 1)
    elseif(${ARM_ARCH_TYPE} STREQUAL "cortex-m4")
        set(STM32G4 1)
    else()
        message( FATAL_ERROR "Failed to find a suitable architecture." )
    endif ()

    configure_file(can/firmware/platform_specific_hal_conf.h.in ${CMAKE_BINARY_DIR}/generated/platform_specific_hal_conf.h)

    include_directories(${CMAKE_BINARY_DIR}/generated/)

endif ()

add_subdirectory(python)

set(LINT_TARGETS "")
add_subdirectory(common)
add_subdirectory(can)
add_subdirectory(motor-control)
add_subdirectory(pipettes)
add_subdirectory(gantry)
add_subdirectory(head)
list(REMOVE_DUPLICATES LINT_TARGETS)

find_package(Clang)

file(GLOB_RECURSE SOURCES_FOR_FORMAT
        ./include/*.hpp
        ./can/*.hpp ./can/*.cpp
        ./common/*.hpp ./common/*.cpp
        ./gantry/*.hpp ./gantry/*.cpp
        ./pipettes/*.hpp ./pipettes/*.cpp
        ./head/*.hpp ./head/*.cpp
        ./motor-control/*.hpp ./motor-control/*.cpp
        )

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
        format
        ALL
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${SOURCES_FOR_FORMAT}
        DEPENDS opentrons_ot3_firmware-format
)

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
        format-ci
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${SOURCES_FOR_FORMAT}
        DEPENDS opentrons_ot3_firmware-format-ci
)

add_custom_target(
        lint
        ALL
        DEPENDS ${LINT_TARGETS} opentrons_ot3_firmware-lint
)
