# Qt Custom User Interface plugin for CoppeliaSim

### Compiling

1. Install required packages for simStubsGen: see simStubsGen's [README](https://github.com/CoppeliaRobotics/include/blob/master/simStubsGen/README.md)
2. Download and install Qt (same version as CoppeliaSim, i.e. 5.15.x)
3. Checkout, compile and install into CoppeliaSim:
```sh
$ git clone https://github.com/CoppeliaRobotics/simUI.git
$ cd simExtUI
$ git checkout coppeliasim-v4.5.0-rev0
$ mkdir -p build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build .
$ cmake --install .
```

NOTE: replace `coppeliasim-v4.5.0-rev0` with the actual CoppeliaSim version you have.
