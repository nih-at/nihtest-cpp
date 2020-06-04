This is nihtest, a testing tool for command line utilities.

Tests are run in a sandbox directory to guarantee a clean separation of the test.

It checks that exit code, standard and error outputs are as expected and compares the files in the sandbox after the run with the expected results.

It is written in C++ 14 and a bit of C and only needs the standard library.

It is documented in man pages: [nihtest(1)](https://raw.githack.com/nih-at/nihtest/master/man/nihtest.html), the config
file format [nihtest.conf(5)](https://raw.githack.com/nih-at/nihtest/master/man/nihtest.conf.html) and the test
case language [nihtest-case(5)](https://raw.githack.com/nih-at/nihtest/master/man/nihtest-case.html).

[![Travis Build Status](https://travis-ci.com/nih-at/nihtest.svg?branch=master)](https://travis-ci.com/nih-at/nihtest)
[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/15uyho5k9tmlrtyc?svg=true)](https://ci.appveyor.com/project/nih-at/nihtest)
