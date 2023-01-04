add_library(motor-utils STATIC 
    motor_hardware_interface.cpp
    stall_check.cpp
    types.cpp    
    utils.cpp)
target_include_directories(motor-utils PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/../../include)

add_coverage(motor-utils)