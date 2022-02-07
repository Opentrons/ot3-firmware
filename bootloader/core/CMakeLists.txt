# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

# generate the constants header that will provide node and message ids
set(IDS_HPP_DIR ${CMAKE_CURRENT_BINARY_DIR}/../../include/bootloader/core)
set(IDS_HPP ${IDS_HPP_DIR}/ids.h)

# while in theory we added the python sources to opentrons_generate_header
# as INTERFACE sources and that should be transitive to the add_custom_command,
# that doesn't actually work - maybe it would if this was a library? - so instead
# we can just pull the list off the target props and explicitly depend on it here
get_property(generator_sources TARGET opentrons_generate_header PROPERTY INTERFACE_SOURCES)

add_custom_command(
        OUTPUT ${IDS_HPP}
        COMMAND opentrons_generate_header ${IDS_HPP} --language c
        DEPENDS opentrons_generate_header ${generator_sources}
)

add_library(bootloader-core
        STATIC
        messages.c
        util.c
        message_handler.c
        ${IDS_HPP}
        )

target_include_directories(bootloader-core
        PUBLIC
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../include)
