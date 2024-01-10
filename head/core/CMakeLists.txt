function(target_head_core_proto TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks_proto.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task.cpp)
        target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
endfunction()

function(target_head_core_rev1 TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks_rev1.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task.cpp)
        target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
endfunction()
