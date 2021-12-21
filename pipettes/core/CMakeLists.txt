function(target_pipette_core TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motion_controller_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_driver_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/move_group_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/move_status_reporter_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/eeprom_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
endfunction()
