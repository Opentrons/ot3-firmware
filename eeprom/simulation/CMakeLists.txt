function(target_eeprom_sim TARGET)
  target_sources(${TARGET} PUBLIC
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/crc16.cpp)
endfunction()
