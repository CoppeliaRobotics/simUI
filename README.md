# Qt Custom User Interface plugin for V-REP

### Compiling

1. Install required packages for [v_repStubsGen](https://github.com/fferri/v_repStubsGen): see v_repStubsGen's [README](external/v_repStubsGen/README.md)
2. Download and install Qt (same version as V-REP, i.e. 5.5.0)
3. Checkout and compile
```
$ git clone --recursive https://github.com/fferri/v_repExtCustomUI.git
$ cmake .
$ cmake --build .
```
you may need to set the `CMAKE_PREFIX_PATH` environment variable to the `lib/cmake` subdirectory of your Qt installation, i.e. `/path/to/Qt/Qt5.9.0/5.9/<platform>/lib/cmake`
