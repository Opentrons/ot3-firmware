# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

#
add_library(bootloader-core
        STATIC
        messages.c
        util.c
        message_handler.c
        update_state.c
        pipette_node_id.c
        ${CMAKE_BINARY_DIR}/common/core/version.c
        )
add_dependencies(bootloader-core generate_version)
target_include_directories(bootloader-core
        PUBLIC
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../include)

add_coverage(bootloader-core)