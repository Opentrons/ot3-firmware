add_library(motor-utils STATIC 
    motor_hardware_interface.cpp
    types.cpp    
    utils.cpp)
target_include_directories(motor-utils PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/../../include)