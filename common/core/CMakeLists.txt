add_custom_target(version
        ${CMAKE_COMMAND} -DGENERATE_VERSION_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/version.c)

add_library(common-core STATIC tool_detection.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.c)
add_dependencies(common-core version)
target_include_directories(common-core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(common-core PUBLIC can-core)


set_target_properties(common-core
        PROPERTIES CXX_STANDARD 20
                   CXX_STANDARD_REQUIRED TRUE)

target_compile_options(common-core
        PRIVATE
        -Wall
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)
