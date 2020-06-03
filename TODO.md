## required for libzip

* implement OS on Windows
* allow embedding in other projects without installing
* improve test failure output
* remove stderr rewrite to remove program name if argv[0] is deterministic on all platforms

## required for release

* README.md, INSTALL.md, etc.
* make intermediary directories when copying file
* test errors in test case description

## enhancements

* make mkdir, ulimit less unix centric
* use readable time format for touch
* implement limits, touch, mkdir
* don't use system() to remove directory
* add timeout directive (and default-timeout in configuration)
* default environment variables in configuration
* unsetenv

## cleanup

