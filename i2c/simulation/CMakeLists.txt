function(target_i2c_simlib TARGET)
  target_sources(${TARGET} PUBLIC
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/i2c_sim.cpp
    )
endfunction()
