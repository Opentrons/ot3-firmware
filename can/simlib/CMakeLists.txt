function(target_can_simlib TARGET)
    target_sources(${TARGET} PUBLIC
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/transport.cpp)
endfunction()
