function(target_i2c_firmware TARGET)
  target_sources(${TARGET} PUBLIC
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/i2c.c
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/i2c_comms.cpp)
endfunction()
