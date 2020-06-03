CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(nihtest
  VERSION 0.1.0
  DESCRIPTION "NiH testing framework"
  HOMEPAGE_URL "https://github.com/nih-at/nihtest"
  LANGUAGES C CXX)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_STANDARD 14)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFiles)
INCLUDE(GNUInstallDirs)

SET(ARCHIVE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})

IF(NOT TARGET dist)
ADD_CUSTOM_TARGET(dist
  COMMAND git config tar.tar.xz.command "xz -c"
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.gz HEAD
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.xz HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
ADD_CUSTOM_TARGET(distcheck
  COMMAND chmod -R u+w ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest 2>/dev/null || true
  COMMAND rm -rf ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
  COMMAND ${CMAKE_COMMAND} -E tar xf ${ARCHIVE_NAME}.tar.gz
  COMMAND chmod -R u-w ${ARCHIVE_NAME}
  COMMAND mkdir ${ARCHIVE_NAME}-build
  COMMAND mkdir ${ARCHIVE_NAME}-dest
  COMMAND cd ${ARCHIVE_NAME}-build && ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=../${ARCHIVE_NAME}-dest ../${ARCHIVE_NAME}
  COMMAND cd ${ARCHIVE_NAME}-build && make -j4
  COMMAND cd ${ARCHIVE_NAME}-build && make test
  COMMAND cd ${ARCHIVE_NAME}-build && make install
#  COMMAND cd ${ARCHIVE_NAME}-build && make uninstall
#  COMMAND if [ `find ${ARCHIVE_NAME}-dest ! -type d | wc -l` -ne 0 ]; then echo leftover files in ${ARCHIVE_NAME}-dest; false; fi
  COMMAND cd ${ARCHIVE_NAME}-build && make clean
  COMMAND chmod -R u+w ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
  COMMAND rm -rf ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
  COMMAND echo "${ARCHIVE_NAME}.tar.gz is ready for distribution."
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
ADD_DEPENDENCIES(distcheck dist)
ENDIF(NOT TARGET dist)

FIND_PROGRAM(MDOCTOOL NAMES mandoc groff)
IF (MDOCTOOL)
  SET(DOCUMENTATION_FORMAT "mdoc" CACHE STRING "Documentation format")
ELSE()
  FIND_PROGRAM(MANTOOL NAMES nroff)
  IF (MANTOOL)
    SET(DOCUMENTATION_FORMAT "man" CACHE STRING "Documentation format")
  ELSE()
    SET(DOCUMENTATION_FORMAT "html" CACHE STRING "Documentation format")
  ENDIF()
ENDIF()

CHECK_FUNCTION_EXISTS(getopt_long HAVE_GETOPT_LONG)
CHECK_FUNCTION_EXISTS(getprogname HAVE_GETPROGNAME)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

SET(HAVE_TEST_EXISTING_FEATURE 1)
SET(HAVE_TEST_MISSING_FEATURE 0)

ENABLE_TESTING()

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(man)
ADD_SUBDIRECTORY(regress)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
