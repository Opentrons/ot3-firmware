function(target_pipettes_core_common TARGET REVISION)
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
    if(${USE_PRESSURE_MOVE})
    add_compile_definitions(-DUSE_PRESSURE_MOVE)
    endif()
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
