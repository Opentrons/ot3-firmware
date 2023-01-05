# This CMakeLists.txt handles compiling all the parts of the can
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.

add_library(can-core STATIC
        can_bus.cpp
        types.cpp
        )

set_target_properties(can-core
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_include_directories(can-core
        PUBLIC
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../include)

target_compile_options(can-core
        PUBLIC
        -Wall
        -Werror
        -Wno-missing-field-initializers
        -Wextra
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        )

add_coverage(can-core)