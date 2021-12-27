function(target_pipettes_core TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/eeprom.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
endfunction()
