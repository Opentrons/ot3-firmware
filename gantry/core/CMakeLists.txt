# This CMakeLists.txt handles compiling all the parts of the gantry
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.
if (NOT GANTRY_AXIS_TYPE)
    SET(GANTRY_AXIS_TYPE "gantry_x" CACHE STRING "Defines the gantry axis: gantry_x or gantry_y")
endif ()


configure_file(./axis_type.cpp.in ./axis_type.cpp)

set(CORE_NONLINTABLE_SRCS
        ${CMAKE_CURRENT_BINARY_DIR}/axis_type.cpp)

add_library(gantry-core STATIC
        ${CORE_NONLINTABLE_SRCS}
        )

set_target_properties(gantry-core
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_include_directories(gantry-core
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(gantry-core
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