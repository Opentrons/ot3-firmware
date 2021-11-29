# This CMakeLists.txt handles compiling all the parts of the motor-control
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.

function(target_ot_motor_control_core TARGET)
  target_sources(${TARGET} PUBLIC
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_interrupt_handler.cpp)
endfunction()

set(LINT_TARGETS ${CMAKE_CURRENT_LIST_DIR}/motor_interrupt_handler.cpp PARENT_SCOPE)
