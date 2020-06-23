set(NIHTEST_SOURCES
    nihtest.cc
    CompareArrays.cc
    CompareFiles.cc
    Configuration.cc
    Exception.cc
    OS.cc
    Parser.cc
    Test.cc
)

if(WIN32)
  list(APPEND NIHTEST_SOURCES OS-Windows.cc)
else()
  list(APPEND NIHTEST_SOURCES
    OS-Unix.cc
    OS-Unix-run.cc
  )
endif()

if(NOT HAVE_GETOPT_LONG)
  list(APPEND NIHTEST_SOURCES getopt_long.c)
endif()
if(NOT HAVE_GETPROGNAME)
  list(APPEND NIHTEST_SOURCES getprogname.c)
endif()

add_executable(nihtest ${NIHTEST_SOURCES})

# for config.h
target_include_directories(nihtest BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

if(NIHTEST_DO_INSTALL)
  install(TARGETS nihtest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

