# Simple library example
An example application that simulates a typical image parsing program
The library simulates a typilcal image decoding library such as libjpeg
Good example for porting to rlbox and testing its features.

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

Application is in `main.cpp`. Library is `lib.c`
