add_executable(nihtest
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
  target_sources(nihtest PRIVATE
    OS-Windows.cc
    )
  target_link_libraries(nihtest PRIVATE bcrypt)
else()
  target_sources(nihtest PRIVATE
    OS-Unix.cc
    OS-Unix-run.cc
  )
endif()

if(NOT HAVE_GETOPT_LONG)
  target_sources(nihtest PRIVATE getopt_long.c )
endif()
if(NOT HAVE_GETPROGNAME)
  target_sources(nihtest PRIVATE getprogname.c)
endif()

# for config.h
target_include_directories(nihtest BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

if(NIHTEST_DO_INSTALL)
  install(TARGETS nihtest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

