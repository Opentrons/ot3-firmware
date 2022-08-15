message("Binary dir is ${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ot3_state_manager_messages.hpp
		COMMENT "Generating ot3_state_manager_messages.hpp"
		DEPENDS ot3_state_manager/generate_state_manager_headers.py
		VERBATIM
		COMMAND poetry run python3 generate_state_manager_headers.py ${CMAKE_CURRENT_BINARY_DIR}/ot3_state_manager_messages.hpp
		)
add_library(ot3_state_manager INTERFACE)
add_dependencies(ot3_state_manager ${CMAKE_CURRENT_BINARY_DIR}/ot3_state_manager_messages.hpp)
target_include_directories(ot3_state_manager INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
