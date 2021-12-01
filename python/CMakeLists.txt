execute_process(
  COMMAND $ENV{SHELL} -c "python -m pip install pipenv==2021.5.29"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND_ERROR_IS_FATAL ANY)

# Make sure we have a venv
execute_process(
  COMMAND $ENV{SHELL} -c "pipenv sync --dev"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND_ERROR_IS_FATAL ANY
  )

add_custom_target(opentrons_ot3_firmware-build-and-test
  COMMAND $ENV{SHELL} -c "pipenv run py.test"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
add_dependencies(build-and-test opentrons_ot3_firmware-build-and-test)

add_custom_target(opentrons_ot3_firmware-format
  COMMAND $ENV{SHELL} -c "pipenv run python -m black opentrons_ot3_firmware tests setup.py"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(opentrons_ot3_firmware-format-ci
  COMMAND $ENV{SHELL} -c "pipenv run python -m black --check opentrons_ot3_firmware tests setup.py"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(opentrons_ot3_firmware-lint
  COMMAND $ENV{SHELL} -c "pipenv run python -m flake8 opentrons_ot3_firmware tests setup.py"
  COMMAND $ENV{SHELL} -c "pipenv run python -m mypy opentrons_ot3_firmware tests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
