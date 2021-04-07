# Qt Custom User Interface plugin for CoppeliaSim

### Compiling

1. Install required packages for [libPlugin](https://github.com/CoppeliaRobotics/libPlugin): see libPlugin's README
2. Download and install Qt (same version as CoppeliaSim, i.e. 5.5.0)
3. Checkout and compile
```
$ git clone --recursive https://github.com/CoppeliaRobotics/simExtCustomUI.git
$ cmake .
$ cmake --build .
```
you may need to set the `CMAKE_PREFIX_PATH` environment variable to the `lib/cmake` subdirectory of your Qt installation, i.e. `/path/to/Qt/Qt5.9.0/5.9/<platform>/lib/cmake`
