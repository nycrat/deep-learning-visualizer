# Deep Learning Visualizer

## Downloading and Building

Make sure to clone recursively for the external library submodules.

```sh
git clone --recursive https://github.com/nycrat/deep-learning-visualizer
```

Run the setup script (`./scripts/setup.sh`) before attempting to build this project.

## Running the tests

```sh
make test
make test BUILD_TYPE=release
```

See [docs/testing.md](docs/testing.md) for how the suite is laid out and how to write
new tests.

## Generating the documentation

```sh
make docs
```

Doxygen writes the site to `docs/html`, starting at `docs/html/index.html`. It is also
published to GitHub Pages on every push; see `.github/workflows/docs.yml`.

## Generating compile_commands.json

To get LSP working properly, use [bear](https://github.com/rizsotto/Bear):

```sh
make clean
bear -- make
```
