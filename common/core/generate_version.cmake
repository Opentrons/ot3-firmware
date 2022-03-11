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
  COMMAND git describe --dirty --long --tags --match=v*
  OUTPUT_VARIABLE GIT_VERSION_RAW
  ERROR_QUIET
  )
if ((NOT DEFINED GIT_VERSION_RAW) OR (GIT_VERSION_RAW STREQUAL ""))
  message(STATUS "No version found from git tag, using version 0, not-version, dirty")
  execute_process(
  COMMAND git rev-parse --short HEAD
  OUTPUT_VARIABLE GIT_SHORTSHA_RAW
  COMMAND_ERROR_IS_FATAL ANY
  )
  string(STRIP ${GIT_SHORTSHA_RAW} GIT_SHORTSHA)

  set(GIT_VERSION "0")
  set(BUILD_IS_EXACT_COMMIT "0")
  set(BUILD_IS_EXACT_VERSION "0")
else()
  # Pull out
  # - the numerical element of the latest tag as the version (match 1)
  # - the number of commits since that tag (match 2)
  # - the sha (match 3)
  # - whether there are uncommitted files (match 4)
  #                    tag (1)  since (2)   sha (3)   uncommitted (4)
  string(REGEX MATCH "v([0-9]+)-([0-9]*)-g([a-f0-9]+)-?(dirty)?" DESCRIBE_MATCH ${GIT_VERSION_RAW})
  set(GIT_VERSION ${CMAKE_MATCH_1})
  set(GIT_SHORTSHA ${CMAKE_MATCH_3})
  if (CMAKE_MATCH_2) # commits-since-tag
    set(BUILD_IS_EXACT_VERSION "0")
  else()
    set(BUILD_IS_EXACT_VERSION "VERSION_BUILD_IS_EXACT_VERSION")
  endif()
  if (CMAKE_MATCH_4)  # dirty
    set(BUILD_IS_EXACT_COMMIT "0")
  else()
    set(BUILD_IS_EXACT_COMMIT "VERSION_BUILD_IS_EXACT_COMMIT")
  endif()
endif()

if ($ENV{CI})
  set(BUILD_IS_FROM_CI "VERSION_BUILD_IS_FROM_CI")
else()
  set(BUILD_IS_FROM_CI "0")
endif()

if (NOT GENERATE_VERSION_TARGET_DIR)
  message(FATAL_ERROR "No target directory set to generate version")
else()
  message(STATUS
    "Generating ${GENERATE_VERSION_TARGET_DIR}/version.c with \n"
    "    version ${GIT_VERSION}\n"
    "    sha ${GIT_SHORTSHA}\n"
    "    flags ${BUILD_IS_FROM_CI}|${BUILD_IS_EXACT_VERSION}|${BUILD_IS_EXACT_COMMIT}")
endif()


configure_file(${CMAKE_CURRENT_LIST_DIR}/version.c.in ${GENERATE_VERSION_TARGET_DIR}/version.c)
