function(target_gantry_core_common TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/utils.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motion_controller_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/motor_driver_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/move_group_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/move_status_reporter_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp)
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
endfunction()

function(target_gantry_core_x TARGET)
    target_gantry_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/axis_type_x.cpp)
endfunction()


function(target_gantry_core_y TARGET)
    target_gantry_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/axis_type_y.cpp)
endfunction()
