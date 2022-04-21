function(target_spi_simlib TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/spi.cpp
            )
endfunction()