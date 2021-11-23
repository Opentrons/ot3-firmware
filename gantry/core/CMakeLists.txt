# Create the lists of SRC files
set(GANTRY_X_CORE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/can_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/axis_type_x.cpp
        CACHE STRING "Source file list for Gantry X"
        FORCE
        )


set(GANTRY_Y_CORE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/can_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/axis_type_y.cpp
        CACHE STRING "Source file list for Gantry Y"
        FORCE
        )
