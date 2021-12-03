# the python subdirectory contains a python module that has data bindings for
# canbus messages, and supports using those bindings to generate id headers.

# any time we're calling python, we need to do it through the shell to rely on
# the shell's location selection, which is where things like pyenv and virtualenvs
# hook (if you just run python directly, cmake will do this on its own and not look
# at shell configuration)
set(SHELL_FOR_PYTHON "/bin/sh" CACHE STRING "shell to use for calling python (supports pyenv)")

# we'll need poetry. poetry is one of those modern utils that wants to live in home
# and get integrated into your shell, and we kind of want to keep poetry's ambit limited
# to this submodule, so we're going to do some fancy footwork to install it locally only

# Change this to change the poetry version
set(POETRY_VERSION 1.1.12)

# we're going to download and install this to stm32-tools along with all the other deps
# even though this is straying a bit far from the concept of "stm32 tools"
set(POETRY_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../stm32-tools/python-poetry")

# since we're calling everything python-related through the shell, we can build command
# strings like this with environment variable prefixing to set them. POETRY_HOME is how
# you tell poetry you installed it somewhere nonstandard.
set(POETRY "POETRY_HOME=${POETRY_PREFIX} ${POETRY_PREFIX}/bin/poetry")

# The poetry installer is just one of those scripts you're supposed to curl | bash, which
# won't really work with proper FetchContent, so we're just going to bodge our own version
# together by making a directory, checking if poetry is already present, and downloading it
# if it's not
if (NOT EXISTS ${POETRY_PREFIX})
  file(MAKE_DIRECTORY ${POETRY_PREFIX})
endif()
# by using ERROR_QUIET, this will give us the version if poetry is installed and an empty
# string if not
execute_process(
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} --version --no-ansi"
  OUTPUT_VARIABLE _poetry_version_string
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
# and now we can see if it's necessary to download (if the version is different, or poetry
# isn't installed) or if we can keep what we have (versions exactly equivalent)
if (NOT "Poetry version ${POETRY_VERSION}" STREQUAL "${_poetry_version_string}")
  message(STATUS "Installing poetry to ${POETRY_PREFIX}")
  file(DOWNLOAD
    "https://install.python-poetry.org"
    ${POETRY_PREFIX}/install-poetry.py)
  # force install poetry to stm32-tools
  set(_poetry_env "POETRY_HOME=${POETRY_PREFIX}")
  # use the specified version and don't ask anything
  set(_poetry_install_args "--version=${POETRY_VERSION} -f -y")
  # how to run the script
  set(_install_poetry "python ${POETRY_PREFIX}/install-poetry.py")
  execute_process(
    COMMAND ${SHELL_FOR_PYTHON} -c "${_poetry_env} ${_install_poetry} ${_poetry_install_args}"
    )
else ()
  message(STATUS "${_poetry_version_string} already installed")
endif()

# poetry install makes all the deps preesnt in the project venv
execute_process(
  COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND_ERROR_IS_FATAL ANY)

# add tests that work alongside the c++ tests
if (NOT ${CMAKE_CROSSCOMPILING})
  add_custom_target(opentrons_ot3_firmware-build-and-test
    COMMAND ${SHELL_FOR_PYTHON} -c "${POETRY} run py.test"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
  add_dependencies(build-and-test opentrons_ot3_firmware-build-and-test)
endif()

# add format and lint alongside c++ format and lint
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

# we generate this opentrons_generate_header script because we're going to tell cmake
# it's an "imported executable target" so we can just use it in add_custom_command
# calls, and that needs an on-disk file rather than command string
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/generate_header_wrapper.in
  ${CMAKE_CURRENT_BINARY_DIR}/opentrons_generate_header
  FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

# using CONFIGURE_DEPENDS here should mean that if any of these files change, then this
# gets rerun. this is not the world's best idea; it's still full of holes and cmake
# docs pretty explicitly say not to do it. the thing you're supposed to do is write all
# the file paths. the thing is though, because we're using poetry, those file paths would
# literally only be used for dependency tracking; if you forgot to list one, nothing would
# fail. doing it this way means that ci will always pick up the new file at least.
file(
  GLOB_RECURSE python_source
  CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_LIST_DIR}/opentrons_ot3_firmware/*.py
  LIST_DIRECTORIES OFF
  )

# an imported executable lets you give a cmake target name to basically any random program,
# and have it be useful in any context where a cmake executable target would be useful - for
# instance, the thing where you can use an executable target as a command in add_custom_command
# and cmake will make sure that command gets called.
add_executable(opentrons_generate_header IMPORTED GLOBAL)
set_target_properties(opentrons_generate_header
  PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/opentrons_generate_header)

# in theory (IN THEORY) adding "interface sources" and explicit dependencies like this should
# pass along dependencies on ${python_source} to anything that depends on this. In practice, I
# think that only works if this is an imported _library_ (see can/core/CMakeLists.txt for what
# we have to do instead)
target_sources(opentrons_generate_header INTERFACE ${python_source})
add_dependencies(opentrons_generate_header ${CMAKE_CURRENT_BINARY_DIR}/opentrons_generate_header)
