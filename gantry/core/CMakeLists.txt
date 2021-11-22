set(COMPILE_OPTS
        -Wall
        -Werror
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy
        -fno-rtti)

add_library(gantry-core-x STATIC
        axis_type_x.cpp
        )

add_library(gantry-core-y STATIC
        axis_type_y.cpp)

set_target_properties(gantry-core-x
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

set_target_properties(gantry-core-y
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)


target_include_directories(gantry-core-x
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_include_directories(gantry-core-y
        PUBLIC ${CMAKE_SOURCE_DIR}/include)


target_compile_options(gantry-core-x PRIVATE ${COMPILE_OPTS})

target_compile_options(gantry-core-y PRIVATE ${COMPILE_OPTS})
