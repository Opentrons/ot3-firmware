add_library(i2c-simlib STATIC
    i2c_sim.cpp)
set_target_properties(i2c-simlib
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(i2c-simlib
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

target_include_directories(i2c-simlib PUBLIC ${CMAKE_SOURCE_DIR}/include)

function(target_i2c_simlib TARGET)
  target_link_libraries(${TARGET} PUBLIC i2c-simlib)
endfunction()
