function(target_gripper_core TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks_g.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tasks_z.cpp)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
    target_link_libraries(${TARGET} PUBLIC common-core)
    target_include_directories(${TARGET} INTERFACE ${CMAKE_SOURCE_DIR}/cpp-utils/include)
endfunction()
