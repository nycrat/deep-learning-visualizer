# Deep Learning Visualizer

## Downloading and Building

Make sure to clone recursively for the external library submodules.

```sh
git clone --recursive https://github.com/nycrat/deep-learning-visualizer
```

Run the setup script (`./scripts/setup.sh`) before attempting to build this project.

## Generating compile_commands.json

To get LSP working properly, use [bear](https://github.com/rizsotto/Bear):

```sh
make clean
bear -- make
```
