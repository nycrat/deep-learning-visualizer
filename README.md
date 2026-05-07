# Deep Learning Visualizer

## Downloading and Building

Make sure to clone recursively for the glfw submodule.

```sh
git clone --recursive https://github.com/nycrat/deep-learning-visualizer
```

Run the cmake and make to get a statically linked library to place in lib/.

Used the [GLAD online tool](https://glad.dav1d.de/) to generate code to load
OpenGL functions at run-time. The specfic configuration can be found
[here](https://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D3.3).

### Setup Script

```sh
cd glfw
cmake -S . -B build
cd build
make
cp src/libglfw3.a ../../lib
cd ../..
make run
```

## Generating compile_commands.json

To get LSP working properly, use [bear](https://github.com/rizsotto/Bear):

```sh
make clean
bear -- make
```
