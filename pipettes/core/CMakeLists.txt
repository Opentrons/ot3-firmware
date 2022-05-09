function(target_pipettes_core_common TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/configs.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/peripheral_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/central_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/linear_motor_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/sensor_tasks.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
endfunction()

function(target_pipettes_core_single TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_single.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_lt.cpp)
endfunction()

function(target_pipettes_core_multi TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_multi.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_lt.cpp)
endfunction()

function(target_pipettes_core_96 TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_96.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gear_motor_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_ht.cpp)
endfunction()

function(target_pipettes_core_384 TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_384.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gear_motor_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_ht.cpp)
endfunction()
