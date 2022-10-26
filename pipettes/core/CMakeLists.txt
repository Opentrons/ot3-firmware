function(target_pipettes_core_common TARGET REVISION)
    if (${TARGET} STREQUAL "pipettes-head")
        target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=NINETY_SIX_CHANNEL)
        target_sources(${TARGET} PUBLIC
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/temporary_head_tasks.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/configs.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_temporary_head.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/head_peripheral_tasks.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/head_sensor_tasks.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/central_tasks.cpp
                )
    else()
        target_sources(${TARGET} PUBLIC
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/configs.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/peripheral_tasks.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/central_tasks.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/linear_motor_tasks.cpp
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gear_motor_tasks.cpp)
        if (${REVISION} STREQUAL "rev1")
            target_sources(${TARGET} PUBLIC
                    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/sensor_tasks_g4.cpp)
        else ()
            target_sources(${TARGET} PUBLIC
                    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/sensor_tasks.cpp)
        endif ()
    endif()
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
endfunction()

function(target_pipettes_core_single TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=SINGLE_CHANNEL)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_low_throughput.cpp)
endfunction()

function(target_pipettes_core_multi TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=EIGHT_CHANNEL)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_low_throughput.cpp)
endfunction()

function(target_pipettes_core_96 TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=NINETY_SIX_CHANNEL)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_high_throughput.cpp)
endfunction()

function(target_pipettes_core_384 TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=THREE_EIGHTY_FOUR_CHANNEL)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_high_throughput.cpp)
endfunction()
