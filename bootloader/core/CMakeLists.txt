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
        )
add_dependencies(bootloader-core generate_version)
target_include_directories(bootloader-core
        PUBLIC
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../include)

target_link_libraries(bootloader-core PUBLIC version-lib)

target_compile_options(bootloader-core
    PRIVATE
    -Wall
    -Werror
    -Wextra
    -Wno-missing-field-initializers
    $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
    $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
    $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
    $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)
add_coverage(bootloader-core)