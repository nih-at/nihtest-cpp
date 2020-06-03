set(TEST_PROGRAMS
  false
  true
  echo
  cat
  getenv
  file
)

foreach(PROGRAM ${TEST_PROGRAMS})
  add_executable(${PROGRAM} ${PROGRAM}.c)
endforeach()

ADD_LIBRARY(ineffective-remove MODULE ineffective-remove.c)
TARGET_LINK_LIBRARIES(ineffective-remove ${CMAKE_DL_LIBS})

# Tests for helper programs
add_test(NAME true COMMAND true)
add_test(NAME false COMMAND false)
set_tests_properties(false PROPERTIES WILL_FAIL TRUE)
add_test(NAME echo COMMAND echo "This test was successful.")
set_tests_properties(echo PROPERTIES PASS_REGULAR_EXPRESSION "This test was successful.")
add_test(NAME cat COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/success.txt)
set_tests_properties(cat PROPERTIES PASS_REGULAR_EXPRESSION "This is a successful test.")
add_test(NAME getenv-defined COMMAND getenv "DEFINED")
set_tests_properties(getenv-defined PROPERTIES ENVIRONMENT DEFINED="This is a successful test.")
set_tests_properties(getenv-defined PROPERTIES PASS_REGULAR_EXPRESSION "DEFINED=\"\"This is a successful test.\"\"")
add_test(NAME getenv-undefined COMMAND getenv "UNDEFINED")
set_tests_properties(getenv-undefined PROPERTIES PASS_REGULAR_EXPRESSION "UNDEFINED=<null>")

# these are not sorted alphabetically but by features
set(NIHTEST_TESTS
  false-fail
  false-pass
  true-fail
  true-pass
  description-pass
  stdout-fail
  stdout-pass
  stderr-fail
  stderr-pass
  stderr-replace-pass
  features-fail
  features-pass
  features-skip
  stdin-pass
  stdin-file-pass
  precheck-fail
  precheck-pass
  precheck-skip
# TODO: fix bug in setenv or test case and enable
#  setenv-pass
  file-del-fail
  file-del-pass
  file-new-fail
  file-new-pass
  file-fail
  file-pass
  preload-pass
  )

# Tests for nihtest itself
foreach(CASE ${NIHTEST_TESTS})
  add_test(NAME ${CASE} COMMAND nihtest ${PROJECT_SOURCE_DIR}/regress/${CASE})
  set_tests_properties(${CASE} PROPERTIES SKIP_RETURN_CODE 77)
  if(${CASE} MATCHES ".*-fail$") 
    set_tests_properties(${CASE} PROPERTIES WILL_FAIL TRUE)
  endif()
endforeach()

configure_file(nihtest.conf.in ${CMAKE_CURRENT_BINARY_DIR}/nihtest.conf @ONLY)
