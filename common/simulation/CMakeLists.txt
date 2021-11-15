add_library(simulation STATIC spi.cpp)

set_target_properties(simulation
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_include_directories(simulation
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(simulation
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