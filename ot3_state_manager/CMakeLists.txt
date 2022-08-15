add_custom_command(
		OUTPUT ot3_state_manager_messages.hpp
		VERBATIM
		COMMAND poetry run python3 generate_state_manager_headers.py ${CMAKE_CURRENT_BINARY_DIR}/ot3_state_manager_messages.hpp
		)
add_library(ot3_state_manager INTERFACE)
target_include_directories(ot3_state_manager INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
