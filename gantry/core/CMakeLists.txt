function(target_gantry_core TARGET AXIS REV)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/utils.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/axis_type_${AXIS}.cpp)
endfunction()
