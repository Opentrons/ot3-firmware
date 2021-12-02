set(SHELL_FOR_PYTHON "/bin/sh" CACHE STRING "shell to use for calling python (supports pyenv)")
set(POETRY_VERSION 1.1.12)
set(POETRY_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../stm32-tools/python-poetry")
set(POETRY "POETRY_HOME=${POETRY_PREFIX} ${POETRY_PREFIX}/bin/poetry")
if (NOT EXISTS ${POETRY_PREFIX})
  file(MAKE_DIRECTORY ${POETRY_PREFIX})
endif()
execute_process(
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} --version --no-ansi"
  OUTPUT_VARIABLE _poetry_version_string
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
if (NOT "Poetry version ${POETRY_VERSION}" STREQUAL "${_poetry_version_string}")
  message(STATUS "Installing poetry to ${POETRY_PREFIX}")
  file(DOWNLOAD
    "https://install.python-poetry.org"
    ${POETRY_PREFIX}/install-poetry.py)
  execute_process(
    COMMAND ${SHELL_FOR_PYTHON} -c "POETRY_HOME=${POETRY_PREFIX} python ${POETRY_PREFIX}/install-poetry.py --version=${POETRY_VERSION} -f -y"
    )
else ()
  message(STATUS "${_poetry_version_string} already installed")
endif()

execute_process(
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND_ERROR_IS_FATAL ANY)

if (NOT ${CMAKE_CROSSCOMPILING})
  add_custom_target(opentrons_ot3_firmware-build-and-test
    COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} run py.test"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
  add_dependencies(build-and-test opentrons_ot3_firmware-build-and-test)
endif()

add_custom_target(opentrons_ot3_firmware-format
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} run black opentrons_ot3_firmware tests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(opentrons_ot3_firmware-format-ci
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} run black --check opentrons_ot3_firmware tests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(opentrons_ot3_firmware-lint
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} run flake8 opentrons_ot3_firmware tests"
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} run mypy opentrons_ot3_firmware tests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/generate_header_wrapper.in
  ${CMAKE_CURRENT_BINARY_DIR}/opentrons_generate_header
  FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

file(
  GLOB_RECURSE python_source
  CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_LIST_DIR}/opentrons_ot3_firmware/*.py
  LIST_DIRECTORIES OFF
  )


add_executable(opentrons_generate_header IMPORTED GLOBAL)
set_target_properties(opentrons_generate_header
  PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/opentrons_generate_header)
target_sources(opentrons_generate_header INTERFACE ${python_source})
add_dependencies(opentrons_generate_header ${CMAKE_CURRENT_BINARY_DIR}/opentrons_generate_header)
