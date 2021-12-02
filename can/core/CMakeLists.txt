# This CMakeLists.txt handles compiling all the parts of the can
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.

# generate the constants header that will provide node and message ids
set(IDS_HPP_DIR ${CMAKE_CURRENT_BINARY_DIR}/../../include/can/core)
set(IDS_HPP ${IDS_HPP_DIR}/ids.hpp)
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
