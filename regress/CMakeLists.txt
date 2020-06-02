set(TEST_PROGRAMS
  false
  true
  echo
)

set(ENV{srcdir} ${PROJECT_SOURCE_DIR}/regress)
foreach(PROGRAM ${TEST_PROGRAMS})
  add_executable(${PROGRAM} ${PROGRAM}.c)
endforeach()

# Tests for helper programs
add_test(NAME true COMMAND true)
add_test(NAME false COMMAND false)
set_tests_properties(false PROPERTIES WILL_FAIL TRUE)
add_test(NAME echo COMMAND echo "This test was successful.")
set_tests_properties(echo PROPERTIES PASS_REGULAR_EXPRESSION "This test was successful.")

set(NIHTEST_TESTS
  false-fail
  false-pass
  true-fail
  true-pass
  )
# subset of NIHTEST_TESTS of those tests which are expected to fail
set(NIHTEST_EXPECTED_FAILS
  false-fail
  true-fail
  )

set(path "$<TARGET_FILE_DIR:nihtest>;$<TARGET_FILE_DIR:false>;$ENV{PATH}")
string(REPLACE ";" "\\;" path "${path}")

# Tests for nihtest itself
foreach(CASE ${NIHTEST_TESTS})
  add_test(NAME ${CASE} COMMAND nihtest ${PROJECT_SOURCE_DIR}/regress/${CASE})
  set_tests_properties(${CASE} PROPERTIES SKIP_RETURN_CODE 77)
  set_tests_properties(${CASE} PROPERTIES ENVIRONMENT "PATH=${path}")
endforeach()
foreach(CASE ${NIHTEST_EXPECTED_FAILS})
  set_tests_properties(${CASE} PROPERTIES WILL_FAIL TRUE)
endforeach()

configure_file(nihtest.conf.in ${CMAKE_CURRENT_BINARY_DIR}/nihtest.conf @ONLY)
