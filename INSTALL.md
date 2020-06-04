nihtest uses [cmake](https://cmake.org) to build.

The basic usage is
```sh
mkdir build
cd build
cmake ..
make
ctest
make install
```

Some useful parameters you can pass to `cmake` with `-Dparameter=value`:

- `CMAKE_INSTALL_PREFIX`: for setting the installation path
- `DOCUMENTATION_FORMAT`: choose one of 'man', 'mdoc', and 'html' for
  the installed documentation (default: decided by cmake depending on
  available tools)
- `NIHTEST_DO_INSTALL`: by default, nihtest installs the binary and
  the man pages. If you use nihtest as a subproject for testing and
  don't want to install any of the files, set this to `ON`.

If you want to compile with custom `CFLAGS`, set them in the environment
before running `cmake`:
```sh
CFLAGS=-DMY_CUSTOM_FLAG cmake ..
```

You can get verbose build output with by passing `VERBOSE=1` to
`make`.

You can also check the [cmake FAQ](https://cmake.org/Wiki/CMake_FAQ).
