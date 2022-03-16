function(target_ot_motor_control_firmware TARGET)
<<<<<<< HEAD
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_control_hardware.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_hardware.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/brushed_motor_hardware.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/brushed_motor_driver_hardware.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
=======
  target_sources(${TARGET} PUBLIC
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_control_hardware.c
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_hardware.cpp)
  target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
>>>>>>> deleted these files
endfunction()
