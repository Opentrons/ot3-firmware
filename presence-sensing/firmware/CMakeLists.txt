function(target_presence_sensing_firmware TARGET)
  target_sources(${TARGET} PUBLIC )
    
  target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
endfunction()
