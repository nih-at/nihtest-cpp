cmake_minimum_required(VERSION 3.12)

project(nihtest
  VERSION 0.1.0
  DESCRIPTION "NiH testing framework"
  HOMEPAGE_URL "https://github.com/nih-at/nihtest"
  LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(GNUInstallDirs)

# added so nihtest can be used as subproject without installing it
option(NIHTEST_DO_INSTALL "Install nihtest and its man pages" ON)
# enable additional linting for development
option(ENABLE_LINTERS "Run linting tools during build" OFF)

if(ENABLE_LINTERS)
  find_program(CLANG_TIDY NAMES clang-tidy)
  find_program(INCLUDE_WHAT_YOU_USE NAMES include-what-you-use iwyu)
  find_program(CPPLINT NAMES cpplint)
  if(CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY} -checks=-*,readability-*)
  endif()
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  endif()
  if(CPPLINT)
    set(CMAKE_CXX_CPPLINT ${CPPLINT})
  endif()
  set(CMAKE_LINK_WHAT_YOU_USE TRUE)
endif()

set(ARCHIVE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})

if(NOT TARGET dist)
add_custom_target(dist
  COMMAND git config tar.tar.xz.command "xz -c"
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.gz HEAD
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.xz HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
add_custom_target(distcheck
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
add_dependencies(distcheck dist)
endif(NOT TARGET dist)

find_program(MDOCTOOL NAMES mandoc groff)
if (MDOCTOOL)
  set(DOCUMENTATION_FORMAT "mdoc" CACHE STRING "Documentation format")
else()
  find_program(MANTOOL NAMES nroff)
  if (MANTOOL)
    set(DOCUMENTATION_FORMAT "man" CACHE STRING "Documentation format")
  else()
    set(DOCUMENTATION_FORMAT "html" CACHE STRING "Documentation format")
  endif()
endif()

check_function_exists(getopt_long HAVE_GETOPT_LONG)
check_function_exists(getprogname HAVE_GETPROGNAME)
check_include_files(unistd.h HAVE_UNISTD_H)

# for testing the "features" keyword
set(HAVE_TEST_EXISTING_FEATURE 1)
set(HAVE_TEST_MISSING_FEATURE 0)

enable_testing()

add_subdirectory(src)
add_subdirectory(man)
add_subdirectory(regress)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
