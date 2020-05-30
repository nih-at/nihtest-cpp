# for config.h
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/..)

SET(NIHTEST_SOURCES
    nihtest.cc
    CompareArrays.cc
    CompareFiles.cc
    Configuration.cc
    Exception.cc
    OS.cc
    Parser.cc
    Test.cc
)

IF(WIN32)
  LIST(APPEND NIHTEST_SOURCES OS-Windows.cc)
ELSE()
  LIST(APPEND NIHTEST_SOURCES
    OS-Unix.cc
    OS-Unix-run.cc
  )
ENDIF()

IF(NOT HAVE_GETOPT_LONG)
  LIST(APPEND NIHTEST_SOURCES getopt_long.c)
ENDIF()
IF(NOT HAVE_GETPROGNAME)
  LIST(APPEND NIHTEST_SOURCES getprogname.c)
ENDIF()

ADD_EXECUTABLE(nihtest ${NIHTEST_SOURCES})

# TODO: make installing optional
INSTALL(TARGETS nihtest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

