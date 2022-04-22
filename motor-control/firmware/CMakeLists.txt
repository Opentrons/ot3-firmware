function(target_ot_motor_control_firmware TARGET)
    target_spi_firmware(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_control_hardware.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/stepper_motor/motor_hardware.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/brushed_motor/brushed_motor_hardware.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/brushed_motor/brushed_motor_driver_hardware.cpp
            ${CMAKE_SOURCE_DIR}/common/firmware/gpio.cpp
            ${CMAKE_SOURCE_DIR}/common/firmware/gpio.c
)
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
endfunction()
