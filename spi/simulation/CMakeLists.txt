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
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_include_directories(spi-simlib PUBLIC ${CMAKE_SOURCE_DIR}/include)

function(target_spi_simlib TARGET)
    target_link_libraries(${TARGET} PUBLIC spi-simlib)
endfunction()
