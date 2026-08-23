#!/bin/bash

PROJECT_ROOT=$(pwd)
EXTERNAL=$PROJECT_ROOT/external

cd $PROJECT_ROOT/glfw
cmake -B build -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_DOCS=OFF --install-prefix=$EXTERNAL
cmake --build build && cmake --build build --target install

cd $PROJECT_ROOT/freetype
cmake -B build -D CMAKE_BUILD_TYPE=Release --install-prefix=$EXTERNAL
cmake --build build && cmake --build build --target install
