function(target_spi_firmware TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/spi.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/spi_comms.cpp)
endfunction()