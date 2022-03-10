# This is callable as a cmake script (e.g. with cmake -P) and will generate a
# version.c that conforms with include/common/core/version.h with version values
# and sha generated from git. This is necessary to allow version.c to change at
# build time as opposed to compile time.
# A variable called GENERATE_VERSION_TARGET_DIR should be defined with the -D
# command line flag when executing this script, as
# cmake -DGENERATE_VERSION_TARGET_DIR=/some/path -P generate_version.cmake
# and the version will be generated in that directory. If not specified, the
# command will fail


execute_process(
  COMMAND git rev-parse --short HEAD
  OUTPUT_VARIABLE GIT_SHORTSHA_RAW
  COMMAND_ERROR_IS_FATAL ANY
  )
execute_process(
  COMMAND git describe --tags --abbrev=0 --match=v*
  OUTPUT_VARIABLE GIT_VERSION_RAW
  ERROR_QUIET
  )
if ((NOT DEFINED GIT_VERSION_RAW) OR (GIT_VERSION_RAW STREQUAL ""))
  message(STATUS "No version found from git tag, using 0")
  set(GIT_VERSION "0")
else()
  string(SUBSTRING ${GIT_VERSION_RAW} 1 -1 GIT_VERSION)
endif()

if (NOT GENERATE_VERSION_TARGET_DIR)
  message(FATAL_ERROR "No target directory set to generate version")
else()
  message(STATUS "Generating ${GENERATE_VERSION_TARGET_DIR}/version.c with version ${GIT_VERSION} and sha ${GIT_SHORTSHA}")
endif()

string(STRIP ${GIT_SHORTSHA_RAW} GIT_SHORTSHA)


configure_file(${CMAKE_CURRENT_LIST_DIR}/version.c.in ${GENERATE_VERSION_TARGET_DIR}/version.c)
