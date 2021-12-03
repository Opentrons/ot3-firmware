# This CMakeLists.txt handles compiling all the parts of the can
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.

# generate the constants header that will provide node and message ids
set(IDS_HPP_DIR ${CMAKE_CURRENT_BINARY_DIR}/../../include/can/core)
set(IDS_HPP ${IDS_HPP_DIR}/ids.hpp)

# while in theory we added the python sources to opentrons_generate_header
# as INTERFACE sources and that should be transitive to the add_custom_command,
# that doesn't actually work - maybe it would if this was a library? - so instead
# we can just pull the list off the target props and explicitly depend on it here
get_property(generator_sources TARGET opentrons_generate_header PROPERTY INTERFACE_SOURCES)
add_custom_command(
        OUTPUT ${IDS_HPP}
        COMMAND opentrons_generate_header ${IDS_HPP}
        DEPENDS opentrons_generate_header ${generator_sources}
        )

add_library(can-core STATIC
        can_bus.cpp
        types.cpp
        ${IDS_HPP}
        )

set_target_properties(can-core
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_include_directories(can-core
        PUBLIC
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../include)

target_compile_options(can-core
        PRIVATE
        -Wall
        -Werror
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy
        -fno-rtti
        )

list(APPEND LINT_TARGETS ${IDS_HPP})
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
