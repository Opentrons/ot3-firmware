# interface library for headers generated from python for messages
# use with target_link_libraries(YOURTARGET state_manager)

set(GENERATED_MESSAGE_HEADER ${CMAKE_BINARY_DIR}/common/simulation/state_manager_messages.hpp)
set(PYSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/state_manager)
set(VENV_PATH ${CMAKE_BINARY_DIR}/.venv)
file(MAKE_DIRECTORY ${VENV_PATH})


add_custom_command(
	OUTPUT ${GENERATED_MESSAGE_HEADER}
	COMMENT "Generating header file and storing to ${GENERATED_MESSAGE_HEADER}"
	VERBATIM
	DEPENDS ${PYSRC_DIR}/messages.py
	COMMAND ${Poetry_EXECUTABLE} run generate-headers ${GENERATED_MESSAGE_HEADER}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_custom_target(state-manager-headers DEPENDS ${GENERATED_MESSAGE_HEADER})
# By depending on this target, you'll be able to include the generated file
# with no path qualifiers (i.e. #include "state_manager_messages.hpp")
add_library(state_manager INTERFACE)
add_dependencies(state_manager state-manager-headers)
target_include_directories(state_manager INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
	state-manager-setup
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} env use 3.8
	COMMAND ${Poetry_EXECUTABLE} install
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-update
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} env use 3.8
	COMMAND ${Poetry_EXECUTABLE} update
	COMMAND ${Poetry_EXECUTABLE} install
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-teardown
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} env info -p | xargs basename | xargs ${Poetry_EXECUTABLE} env remove
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-build
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} build
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-clean
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND rm -rf build dist .coverage coverage.xml '*.egg-info' '__pycache__' '*.pyc' '.mypy_cache'
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-test
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND ${Poetry_EXECUTABLE} run py.test tests --cov=state_manager --cov-report term-missing:skip-covered --cov-report xml:coverage.xml
	WORKING_DIRECTORY ../../state_manager
)

add_custom_target(
	state-manager-lint
	COMMENT "Running linting on state_manager project"
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
    COMMAND ${Poetry_EXECUTABLE} run python -m mypy state_manager tests/ generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m isort --check state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m black --check --line-length 88 state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m flake8 state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m mdformat --check README.md
	WORKING_DIRECTORY ../../state_manager
)


add_custom_target(
	state-manager-format
	# Have to wrap this isort command in a bash -c for some reason because otherwise it fails on the -m for no reason
	# other than cmake is dumb sometimes.
	COMMAND ${Poetry_EXECUTABLE} config virtualenvs.path ${VENV_PATH} --local
	COMMAND bash -c "${Poetry_EXECUTABLE} run python -m isort state_manager tests generate_state_manager_headers.py"
	COMMAND ${Poetry_EXECUTABLE} run python -m black state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m mdformat README.md
	WORKING_DIRECTORY ../../state_manager
)

add_dependencies(state-manager-test state-manager-setup)
add_dependencies(state-manager-lint state-manager-setup)
add_dependencies(state-manager-format state-manager-setup)
unset(ENV{POETRY_VIRTUALENVS_PATH})
