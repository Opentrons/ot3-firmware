## Create the lists of SRC files
#set(GANTRY_X_CORE_SRC
#        ${CMAKE_CURRENT_SOURCE_DIR}/can_task.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/axis_type_x.cpp
#        CACHE STRING "Source file list for Gantry X"
#        FORCE
#        )
#
#
#set(GANTRY_Y_CORE_SRC
#        ${CMAKE_CURRENT_SOURCE_DIR}/can_task.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/axis_type_y.cpp
#        CACHE STRING "Source file list for Gantry Y"
#        FORCE
#        )

function(target_gantry_core_common TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/can_task.cpp
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/utils.cpp)
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
