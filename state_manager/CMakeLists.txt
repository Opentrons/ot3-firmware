# interface library for headers generated from python for messages
# use with target_link_libraries(YOURTARGET state_manager)

set_property(
	DIRECTORY
	APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/coverage.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/.coverage"
    "${CMAKE_CURRENT_SOURCE_DIR}/.mypy_cache"
    "${CMAKE_CURRENT_SOURCE_DIR}/.pytest_cache"
    "${CMAKE_CURRENT_SOURCE_DIR}/build"
    "${CMAKE_CURRENT_SOURCE_DIR}/dist"
    "${CMAKE_CURRENT_SOURCE_DIR}/__pycache__"
)

set(MESSAGE_IDS_PATH ${CMAKE_BINARY_DIR}/common/simulation/message_ids.hpp)
set(MOVE_MESSAGE_HW_IDS_PATH ${CMAKE_BINARY_DIR}/common/simulation/move_message_hw_ids.hpp)
set(DIRECTION_PATH ${CMAKE_BINARY_DIR}/common/simulation/direction.hpp)
set(SYNC_PIN_STATE_PATH ${CMAKE_BINARY_DIR}/common/simulation/sync_pin_state.hpp)
set(PYSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/state_manager)
set(VIRTUAL_ENVIRONMENTS_ROOT_PATH ${CMAKE_BINARY_DIR}/.venv)
file(MAKE_DIRECTORY ${VIRTUAL_ENVIRONMENTS_ROOT_PATH})
# Have to run poetry command to see if venv exists.
execute_process(
	COMMAND ${Poetry_EXECUTABLE} --no-ansi env info --path
	OUTPUT_VARIABLE STATE_MANAGER_VIRTUALENV_DIR
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(STATE_MANAGER_VIRTUALENV_ACTIVATE_FILE ${STATE_MANAGER_VIRTUALENV_DIR}/bin/activate)
add_custom_command(
	OUTPUT ${STATE_MANAGER_VIRTUALENV_ACTIVATE_FILE}
	DEPENDS pyproject.toml poetry.lock
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} --no-ansi env use 3.10
	COMMAND ${Poetry_EXECUTABLE} --no-ansi install
)

add_custom_command(
	OUTPUT ${MESSAGE_IDS_PATH}
	COMMENT "Generating header file and storing to ${MESSAGE_IDS_PATH}"
	VERBATIM
	DEPENDS ${PYSRC_DIR}/messages.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run generate-headers message-ids ${MESSAGE_IDS_PATH}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)


add_custom_command(
	OUTPUT ${MOVE_MESSAGE_HW_IDS_PATH}
	COMMENT "Generating header file and storing to ${MOVE_MESSAGE_HW_IDS_PATH}"
	VERBATIM
	DEPENDS ${PYSRC_DIR}/messages.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run generate-headers move-message-hardware-ids ${MOVE_MESSAGE_HW_IDS_PATH}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)


add_custom_command(
	OUTPUT ${DIRECTION_PATH}
	COMMENT "Generating header file and storing to ${DIRECTION_PATH}"
	VERBATIM
	DEPENDS ${PYSRC_DIR}/messages.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run generate-headers direction ${DIRECTION_PATH}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)


add_custom_command(
	OUTPUT ${SYNC_PIN_STATE_PATH}
	COMMENT "Generating header file and storing to ${SYNC_PIN_STATE_PATH}"
	VERBATIM
	DEPENDS ${PYSRC_DIR}/messages.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run generate-headers sync-pin-state ${SYNC_PIN_STATE_PATH}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

add_custom_target(state-manager-headers DEPENDS ${SYNC_PIN_STATE_PATH} ${MESSAGE_IDS_PATH} ${MOVE_MESSAGE_HW_IDS_PATH} ${DIRECTION_PATH})
# By depending on this target, you'll be able to include the generated file
# with no path qualifiers (i.e. #include "state_manager_messages.hpp")
add_library(state_manager INTERFACE)
add_dependencies(state_manager state-manager-headers)
target_include_directories(state_manager INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
	state-manager-setup
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} --no-ansi env use 3.10
	COMMAND ${Poetry_EXECUTABLE} --no-ansi install
	WORKING_DIRECTORY ../../state_manager
)

add_dependencies(state-manager-headers state-manager-setup)

add_custom_target(
	state-manager-update
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} --no-ansi env use 3.10
	COMMAND ${Poetry_EXECUTABLE} --no-ansi update
	COMMAND ${Poetry_EXECUTABLE} --no-ansi install
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-teardown
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} --no-ansi env info -p | xargs basename | xargs ${Poetry_EXECUTABLE} --no-ansi env remove
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-build
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} --no-ansi build --format wheel
	WORKING_DIRECTORY ../../state_manager
	DEPENDS ${STATE_MANAGER_VIRTUALENV_ACTIVATE_FILE}
)

add_custom_target(
	state-manager-test
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run py.test tests --cov=state_manager --cov-report term-missing:skip-covered --cov-report xml:coverage.xml
	WORKING_DIRECTORY ../../state_manager
	DEPENDS ${STATE_MANAGER_VIRTUALENV_ACTIVATE_FILE}
)

add_custom_target(
	state-manager-lint
	COMMENT "Running linting on state_manager project"
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
    COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m mypy state_manager tests/ generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m isort --check state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m black --check --line-length 88 state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m flake8 state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m mdformat --check README.md
	WORKING_DIRECTORY ../../state_manager
	DEPENDS ${STATE_MANAGER_VIRTUALENV_ACTIVATE_FILE}
)


add_custom_target(
	state-manager-format
	# Have to wrap this isort command in a bash -c for some reason because otherwise it fails on the -m for no reason
	# other than cmake is dumb sometimes.
	COMMAND ${Poetry_EXECUTABLE} --no-ansi config virtualenvs.path ${VIRTUAL_ENVIRONMENTS_ROOT_PATH} --local
	COMMAND bash -c "${Poetry_EXECUTABLE} --no-ansi run python -m isort state_manager tests generate_state_manager_headers.py"
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m black state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} --no-ansi run python -m mdformat README.md
	WORKING_DIRECTORY ../../state_manager
	DEPENDS ${STATE_MANAGER_VIRTUALENV_ACTIVATE_FILE}
)
