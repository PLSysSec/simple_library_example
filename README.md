# Simple library example

An example application that simulates a typical image parsing program
The library simulates a typical image decoding library such as libjpeg
This is meant to serve as a toy example for RLBox.

You can find the RLBox tutorial [here](https://docs.rlbox.dev).
After reading this tutorial you should be able to migrate this application to
using sandboxed libraries using RLBox.

Build with

```bash
mkdir build
cd build
cmake ../
cmake --build . -DCMAKE_BUILD_TYPE=Debug
```

Run with

```bash
./img_app
```

Application is in `main.cpp`. Library is `lib.c`.

The solution is in `solution.cpp` and can be run with

```bash
./img_app_solution
```

