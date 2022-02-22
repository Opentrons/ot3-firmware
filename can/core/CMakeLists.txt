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
