set(TEST_PROGRAMS
  false
  true
  echo
  cat
)

foreach(PROGRAM ${TEST_PROGRAMS})
  add_executable(${PROGRAM} ${PROGRAM}.c)
endforeach()

# Tests for helper programs
add_test(NAME true COMMAND true)
add_test(NAME false COMMAND false)
set_tests_properties(false PROPERTIES WILL_FAIL TRUE)
add_test(NAME echo COMMAND echo "This test was successful.")
set_tests_properties(echo PROPERTIES PASS_REGULAR_EXPRESSION "This test was successful.")
add_test(NAME cat COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/success.txt)
set_tests_properties(cat PROPERTIES PASS_REGULAR_EXPRESSION "This is a successful test.")

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
