# interface library for headers generated from python for messages
# use with target_link_libraries(YOURTARGET ot3_state_manager)

set(GENERATED_MESSAGE_HEADER ${CMAKE_BINARY_DIR}/common/simulation/ot3_state_manager_messages.hpp)
message("GENERATED_MESSAGE_HEADER: ${GENERATED_MESSAGE_HEADER}")
set(PYSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ot3_state_manager)
add_custom_command(
	OUTPUT ${GENERATED_MESSAGE_HEADER}
	COMMENT "Generating ot3_state_manager_messages.hpp"
	VERBATIM
	DEPENDS ${PYSRC_DIR}/messages.py
	COMMAND ${Poetry_EXECUTABLE} run python3 generate_state_manager_headers.py ${GENERATED_MESSAGE_HEADER}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_custom_target(state-manager-headers DEPENDS ${GENERATED_MESSAGE_HEADER})
# By depending on this target, you'll be able to include the generated file
# with no path qualifiers (i.e. #include "ot3_state_manager_messages.hpp")
add_library(ot3_state_manager INTERFACE)
add_dependencies(ot3_state_manager state-manager-headers)
target_include_directories(ot3_state_manager INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
	ot3-state-manager-setup
	COMMAND ${Poetry_EXECUTABLE} env use 3.8
	COMMAND ${Poetry_EXECUTABLE} update
	COMMAND ${Poetry_EXECUTABLE} install
	WORKING_DIRECTORY ../../ot3_state_manager
)

add_custom_target(
	ot3-state-manager-teardown
	COMMAND ${Poetry_EXECUTABLE} env info -p | xargs basename | xargs ${Poetry_EXECUTABLE} env remove
	WORKING_DIRECTORY ../../ot3_state_manager
)

add_custom_target(
	ot3-state-manager-build
	COMMAND ${Poetry_EXECUTABLE} build
	WORKING_DIRECTORY ../../ot3_state_manager
)

add_custom_target(
	ot3-state-manager-clean
	COMMAND rm -rf build dist .coverage coverage.xml '*.egg-info' '__pycache__' '*.pyc' '.mypy_cache'
	WORKING_DIRECTORY ../../ot3_state_manager
)

add_custom_target(
	ot3-state-manager-test
	COMMAND ${Poetry_EXECUTABLE} run py.test tests --cov=ot3_state_manager --cov-report term-missing:skip-covered --cov-report xml:coverage.xml
	WORKING_DIRECTORY ../../ot3_state_manager
)

add_custom_target(
	ot3-state-manager-lint
	COMMENT "Running linting on ot3_state_manager project"
    COMMAND ${Poetry_EXECUTABLE} run python -m mypy ot3_state_manager tests/ generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m isort --check ot3_state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m black --check --line-length 88 ot3_state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m flake8 ot3_state_manager tests generate_state_manager_headers.py
	COMMAND ${Poetry_EXECUTABLE} run python -m mdformat --check README.md
	WORKING_DIRECTORY ../../ot3_state_manager
)


add_custom_target(
	ot3-state-manager-format
	# Have to wrap this isort command in a bash -c for some reason because otherwise it fails on the -m for no reason
	# other than cmake is dumb sometimes.
	bash -c "${Poetry_EXECUTABLE} run python -m isort ot3_state_manager tests generate_state_manager_headers.py"
	${Poetry_EXECUTABLE} run python -m black ot3_state_manager tests generate_state_manager_headers.py
	${Poetry_EXECUTABLE} run python -m mdformat README.md
	WORKING_DIRECTORY ../../ot3_state_manager
)

add_dependencies(ot3-state-manager-test ot3-state-manager-setup)
add_dependencies(ot3-state-manager-lint ot3-state-manager-setup)
add_dependencies(ot3-state-manager-format ot3-state-manager-setup)
