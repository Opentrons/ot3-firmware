function(target_pipettes_core_common TARGET REVISION)
    if(${USE_SENSOR_MOVE})
    target_compile_definitions(${TARGET} PUBLIC USE_SENSOR_MOVE)
    endif()
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/configs.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/peripheral_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/central_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/linear_motor_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gear_motor_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/sensor_tasks.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
    target_include_directories(${TARGET} INTERFACE ${CMAKE_SOURCE_DIR}/cpp-utils/include)
endfunction()

function(target_pipettes_core_single TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=SINGLE_CHANNEL)
    if(${USE_SENSOR_MOVE})
    target_compile_definitions(${TARGET} PUBLIC P_BUFF_SIZE=2800)
    endif()
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_low_throughput.cpp)
endfunction()

function(target_pipettes_core_multi TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=EIGHT_CHANNEL)
    if(${USE_SENSOR_MOVE})
    target_compile_definitions(${TARGET} PUBLIC P_BUFF_SIZE=1800)
    target_compile_definitions(${TARGET} PUBLIC USE_TWO_BUFFERS=true)
    endif()
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_low_throughput.cpp)
endfunction()

function(target_pipettes_core_96 TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=NINETY_SIX_CHANNEL)
    if(${USE_SENSOR_MOVE})
    target_compile_definitions(${TARGET} PUBLIC P_BUFF_SIZE=900)
    target_compile_definitions(${TARGET} PUBLIC USE_TWO_BUFFERS=true)
    endif()
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_high_throughput.cpp)
endfunction()

function(target_pipettes_core_384 TARGET REVISION)
    target_pipettes_core_common(${TARGET} ${REVISION})
    target_compile_definitions(${TARGET} PUBLIC PIPETTE_TYPE_DEFINE=THREE_EIGHTY_FOUR_CHANNEL)
    if(${USE_SENSOR_MOVE})
    target_compile_definitions(${TARGET} PUBLIC P_BUFF_SIZE=1500)
    target_compile_definitions(${TARGET} PUBLIC USE_TWO_BUFFERS=true)
    endif()
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task_high_throughput.cpp)
endfunction()
