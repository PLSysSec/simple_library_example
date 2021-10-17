# Simple library example

An example application that simulates a typical image parsing program
The library simulates a typical image decoding library such as libjpeg
This is meant to serve as a toy example for RLBox.

You can find the RLBox tutorial [here](https://docs.rlbox.dev).
After reading this tutorial you should be able to migrate this application to
using sandboxed libraries using RLBox.

This library has been tested on Linux, Windows and Mac.

Build with

```bash
cmake -S ./ -B ./build
cmake --build ./build --parallel
```

Run with

```bash
cmake --build ./build --target run
```

Application is in `main.cpp`. Library is `lib.c`.

The solution is in `solution.cpp` and can be run with

```bash
cmake --build ./build --target run_solution
```

