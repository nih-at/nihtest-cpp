**The C++ version was never finished and is superseded by the [Python version](https://github.com/nih-at/nihtest/).**

This is nihtest, a testing tool for command line utilities.

Tests are run in a sandbox directory to guarantee a clean separation of the test.

It checks that exit code, standard and error outputs are as expected and compares the files in the sandbox after the run with the expected results.

It is written in C++ 14 and a bit of C and only needs the standard library.

It is documented in man pages: [nihtest(1)](https://raw.githack.com/nih-at/nihtest-cpp/master/man/nihtest.html), the config
file format [nihtest.conf(5)](https://raw.githack.com/nih-at/nihtest-cpp/master/man/nihtest.conf.html) and the test
case language [nihtest-case(5)](https://raw.githack.com/nih-at/nihtest-cpp/master/man/nihtest-case.html).

[![Github Actions Build Status](https://github.com/nih-at/nihtest-cpp/workflows/build/badge.svg)](https://github.com/nih-at/nihtest-cpp/actions?query=workflow%3Abuild)
