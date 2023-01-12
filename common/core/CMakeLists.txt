add_custom_target(generate_version
        ${CMAKE_COMMAND} -DGENERATE_VERSION_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/version.c)

add_library(version-lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/version.c)

target_compile_options(version-lib
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
target_include_directories(version-lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_dependencies(version-lib generate_version)

add_library(common-core STATIC ${CMAKE_CURRENT_BINARY_DIR}/version.c)
target_include_directories(common-core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(common-core PUBLIC can-core version-lib)


set_target_properties(common-core
        PROPERTIES CXX_STANDARD 20
                   CXX_STANDARD_REQUIRED TRUE)

target_compile_options(common-core
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

add_coverage(common-core)
