add_library(spi-simlib STATIC
    spi.cpp)
set_target_properties(spi-simlib
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(spi-simlib
        PUBLIC
        -Wall
        -Werror
        -Wextra
        -Wno-missing-field-initializers
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy)

target_include_directories(spi-simlib PUBLIC ${CMAKE_SOURCE_DIR}/include)

function(target_spi_simlib TARGET)
    target_link_libraries(${TARGET} PUBLIC spi-simlib)
endfunction()
