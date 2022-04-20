function(target_pipettes_core_common TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/configs.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
endfunction()

function(target_pipettes_core_single TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_single.cpp)
endfunction()

function(target_pipettes_core_multi TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_multi.cpp)
endfunction()

function(target_pipettes_core_96 TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_96.cpp)
endfunction()

function(target_pipettes_core_384 TARGET)
    target_pipettes_core_common(${TARGET})
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pipette_type_384.cpp)
endfunction()
