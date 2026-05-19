#!/bin/bash

cd glfw
cmake -S . -B build
cd build
make
cp src/libglfw3.a ../../lib
cd ../..
