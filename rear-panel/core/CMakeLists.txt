function(target_rear_panel_core_rev1 TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/queues.cpp
        )
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
endfunction()
