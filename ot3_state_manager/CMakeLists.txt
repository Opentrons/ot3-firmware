# interface library for headers generated from python for messages
# use with target_link_libraries(YOURTARGET ot3_state_manager)
set(GENERATED_MESSAGE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/ot3_state_manager_messages.hpp)
set(PYSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ot3_state_manager)
add_custom_command(
		OUTPUT ${GENERATED_MESSAGE_HEADER}
		COMMENT "Generating ot3_state_manager_messages.hpp"
		VERBATIM
		DEPENDS ${PYSRC_DIR}/messages.py
		COMMAND poetry run python3 generate_state_manager_headers.py ${GENERATED_MESSAGE_HEADER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
add_custom_target(state-manager-headers DEPENDS ${GENERATED_MESSAGE_HEADER})
# By depending on this target, you'll be able to include the generated file
# with no path qualifiers (i.e. #include "ot3_state_manager_messages.hpp")
add_library(ot3_state_manager INTERFACE)
add_dependencies(ot3_state_manager state-manager-headers)
target_include_directories(ot3_state_manager INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
