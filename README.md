# Deep Learning Visualizer

An interactive neural network. Paint a digit in a 28×28 grid with the mouse,
press <kbd>space</kbd>, and the model tells you which digit it thinks you drew.
Press <kbd>t</kbd> to train it on MNIST and watch the cost fall, then press
<kbd>y</kbd> to measure its accuracy.

The point is to make backpropagation and weight initialization something you
can poke at rather than read about. Every prediction is computed live from the
pixels under your cursor.

## Controls

| Input                   | Action                                               |
| ----------------------- | ---------------------------------------------------- |
| Left mouse button, drag | Paint the digit                                      |
| <kbd>space</kbd>        | Predict the drawn digit, printed to the terminal     |
| <kbd>r</kbd>            | Clear the grid                                       |
| <kbd>t</kbd>            | Train on MNIST                                       |
| <kbd>y</kbd>            | Report accuracy on the MNIST test set                |
| <kbd>s</kbd>            | Save the current weights to `data/mnist/trained.mlp` |

Predictions and training progress are printed to the terminal that launched the
program, not drawn in the window.

## Requirements

The project targets a desktop OpenGL 3.3 core profile, so it needs a GPU and
driver that can provide one. It builds on macOS and Linux, and CI runs it on
Ubuntu.

Building requires a C++23 compiler, CMake, and a few system libraries.

### macOS

Needs Xcode or the Command Line Tools for a C++23 compiler, plus CMake.

```sh
xcode-select --install
brew install cmake
```

macOS is detected automatically and the required system frameworks are linked
for you. No OpenGL library needs to be installed, since the system provides it.

### Linux (Ubuntu and Debian)

```sh
sudo apt-get install -y g++-14 cmake libwayland-dev libxkbcommon-dev xorg-dev
```

`libwayland-dev`, `libxkbcommon-dev`, and `xorg-dev` are what GLFW needs to
create a window on Linux.

## Building

Clone with the submodules, since GLFW and FreeType are pulled in as git
submodules and both need to be compiled before the project can link against
them.

```sh
git clone --recursive https://github.com/nycrat/deep-learning-visualizer
cd deep-learning-visualizer
```

Then build the dependencies. This installs GLFW and FreeType into `external/`,
which is where the build looks for them.

```sh
./scripts/setup.sh
```

The MNIST files are already committed under `data/mnist/`, so there is nothing
else to download.

Finally, build and run:

```sh
make
make run
```

`make` produces a debug build in `bin/debug/`. Set `BUILD_TYPE=release` for an
optimized build in `bin/release/`:

```sh
make BUILD_TYPE=release
```

Release builds use `-O3 -march=native`, so they are fast but not portable
between machines.

## Running the tests

```sh
make test
make test BUILD_TYPE=release
```

See [docs/testing.md](docs/testing.md) for how the suite is laid out and how to
write new tests.

## Generating the documentation

```sh
make docs
```

Doxygen writes the site to `docs/html`, starting at `docs/html/index.html`. It
is also published to GitHub Pages on every push; see
`.github/workflows/docs.yml`.

## Generating compile_commands.json

To get LSP working properly, use [bear](https://github.com/rizsotto/Bear):

```sh
make clean
bear -- make
```
