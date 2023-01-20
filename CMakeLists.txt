# CMake 3.19 is the first to include presets
# (https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED ENV{PRESET_IN_USE})
    message(FATAL_ERROR "Use a preset. Use cmake --list-presets to show available presets.")
endif ()

project("OT3Firmware"
        LANGUAGES CXX C ASM
        DESCRIPTION "Firmware for OT-3 System")

set(CMAKE_MODULE_PATH 
        "${PROJECT_SOURCE_DIR}/cmake" 
        "${PROJECT_SOURCE_DIR}/cmake/codecov/cmake"
        ${CMAKE_MODULE_PATH})

set(ROOT_BOARD_FILE_PATH "${PROJECT_SOURCE_DIR}/include/firmware/" ${ROOT_BOARD_FILE_PATH})

if (NOT ${CMAKE_CROSSCOMPILING})
    enable_testing()
    add_custom_target(build-and-test)

    find_package(codecov)

    # We can safely ignore test code and stm32-tools imports
    list(APPEND LCOV_REMOVE_PATTERNS 
        "'${PROJECT_SOURCE_DIR}/stm32-tools/*'"
        "'${PROJECT_SOURCE_DIR}/**/tests/*.cpp'"
        "'${PROJECT_SOURCE_DIR}/**/simulator/*.cpp'")
else()
    function(add_coverage TARGET)
    endfunction()

    function(coverage_evaluate)
    endfunction()
endif ()

find_package(Clang)

# Set a default build type if none was specified
# From https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "RelWithDebInfo")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "RelWithDebInfo" "Debug" "MinSizeRel")
endif ()
message("Build type is ${CMAKE_BUILD_TYPE}")


if (${CMAKE_CROSSCOMPILING})
    find_package(CrossGCC)
    find_package(OpenOCD)

    set(STM32L5 0)
    set(STM32G4 0)
    if (${ARM_ARCH_TYPE} STREQUAL "cortex-m33")
        set(STM32L5 1)
    elseif (${ARM_ARCH_TYPE} STREQUAL "cortex-m4")
        set(STM32G4 1)
    else ()
        message(FATAL_ERROR "Failed to find a suitable architecture.")
    endif ()

    configure_file(common/firmware/platform_specific_hal_conf.h.in ${CMAKE_BINARY_DIR}/generated/platform_specific_hal_conf.h)
    configure_file(common/firmware/platform_specific_hal.h.in ${CMAKE_BINARY_DIR}/generated/platform_specific_hal.h)

    include_directories(${CMAKE_BINARY_DIR}/generated/)
else ()
    find_package(Boost 1.71.0)
endif ()

set(LINT_TARGETS "")
add_subdirectory(common)
add_subdirectory(can)
add_subdirectory(spi)
add_subdirectory(i2c)
add_subdirectory(motor-control)
add_subdirectory(sensors)
add_subdirectory(pipettes)
add_subdirectory(gantry)
add_subdirectory(head)
add_subdirectory(gripper)
add_subdirectory(eeprom)
add_subdirectory(bootloader)
list(REMOVE_DUPLICATES LINT_TARGETS)

message(VERBOSE "Checking if cross-compiling.")
if(NOT ${CMAKE_CROSSCOMPILING})
	message(VERBOSE "Not cross-compiling. Adding state_manager")
	find_package(Poetry 1.3.2)
	add_subdirectory(state_manager)
else ()
	message(VERBOSE "You are cross-compiling. Not adding state_manager")
endif ()

# c++ linting is here but python linting is in the subdir
file(GLOB_RECURSE SOURCES_FOR_FORMAT
        ./include/*.hpp
        ./can/*.hpp ./can/*.cpp
        ./common/*.hpp ./common/*.cpp
        ./gantry/*.hpp ./gantry/*.cpp
        ./pipettes/*.hpp ./pipettes/*.cpp
        ./head/*.hpp ./head/*.cpp
        ./motor-control/*.hpp ./motor-control/*.cpp
        ./gripper/*.hpp ./gripper/*.cpp
        ./bootloader/*.hpp ./bootloader/*.cpp
        ./i2c/*.hpp ./i2c/*.cpp
        ./sensors/*.hpp ./sensors/*.cpp
        ./spi/*.hpp ./spi/*.cpp
        ./eeprom/*.hpp ./eeprom/*.cpp
        )

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

add_custom_target(
        lint
        ALL
        DEPENDS ${LINT_TARGETS})

# any time we're calling python, we need to do it through the shell to rely on
# the shell's location selection, which is where things like pyenv and virtualenvs
# hook (if you just run python directly, cmake will do this on its own and not look
# at shell configuration)
set(SHELL_FOR_PYTHON "/bin/sh" CACHE STRING "shell to use for calling python (supports pyenv)")
# find the constants.py file. if this doesn't find where you've checked out the monorepo, you
# can override the path in the cmake cache
message(CHECK_START "Finding opentrons_hardware python package")
find_path(OPENTRONS_HARDWARE_PATH
        opentrons_hardware.cmakefind
        DOC "The location of the opentrons_hardware package containing definitions for canbus header generation"
        PATHS ${CMAKE_SOURCE_DIR}/../opentrons $ENV{HOME}/opentrons /opentrons
        PATH_SUFFIXES hardware/opentrons_hardware
        NO_DEFAULT_PATH)

if (NOT OPENTRONS_HARDWARE_PATH)
    message(CHECK_FAIL "not found. You will not be able to generate updated CANbus constant "
            "headers, but otherwise everything will work. If you want to be able to generate headers, "
            "check out the opentrons monorepo as a sibling to this directory or set the cache variable "
            "OPENTRONS_HARDARE_PATH (using a CMake cache editor or by passing "
            "-DOPENTRONS_HARDWARE_PATH=/path/to/opentrons_hardware the next time you configure)")
else ()
    message(CHECK_PASS
            "${OPENTRONS_HARDWARE_PATH}. Update CANbus headers by running cmake --build --preset=<any> --target update-headers")
endif ()

add_custom_target(update-headers
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
        COMMAND ${SHELL_FOR_PYTHON} -c "python ./generate_header.py ${CMAKE_SOURCE_DIR}/include/can/core/ids.hpp --source-dir=${OPENTRONS_HARDWARE_PATH} --language=c++"
        COMMAND ${SHELL_FOR_PYTHON} -c "python ./generate_header.py ${CMAKE_SOURCE_DIR}/include/bootloader/core/ids.h --source-dir=${OPENTRONS_HARDWARE_PATH} --language=c")

coverage_evaluate()
