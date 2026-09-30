# Writing tests

The test suite is [Catch2](https://github.com/catchorg/Catch2) v3, vendored as the
single-file amalgamated distribution in `tests/catch_amalgamated.h` and
`tests/catch_amalgamated.cpp`.

## Running the suite

```sh
make test                    # debug build
make test BUILD_TYPE=release # release build
```

Both targets build `bin/debug/test` or `bin/release/test` and run it. The suite has
no arguments to pass through in the Makefile, but the binary is an ordinary Catch2
runner, so you can select test cases directly:

```sh
bin/debug/test "Test layer*"              # by name, wildcard aware
bin/debug/test "Test layer constructor*"  # narrow it down
bin/debug/test ~"[.]"                     # exclude hidden cases
bin/debug/test -?                         # list the options Catch2 supports
```

A failing assertion prints the test name, the file, the line, the expanded
expression, and the values that were compared:

```
-------------------------------------------------------------------------------
Test layer constructor zero initializes its values
-------------------------------------------------------------------------------
tests/models/layer_test.cpp:42
...............................................................................

tests/models/layer_test.cpp:46: FAILED:
  CHECK( l.activations.isZero(0.0f) )
with expansion:
  false
```

Note: Because `layer` and `mnist` seed their weights from `std::random_device`,
the seed is printed on every run. A failure that only happens for one seed is a
real bug in the test or the code, not something to re-run until it passes.

## Layout

Test files mirror the directory they cover under `src/`:

| Source                                  | Test                                             |
| --------------------------------------- | ------------------------------------------------ |
| `src/shared/math.h`                     | `tests/shared/math_test.cpp`                     |
| `src/shared/idx_matrix.h`               | `tests/shared/idx_matrix_test.cpp`               |
| `src/models/layer.h`                    | `tests/models/layer_test.cpp`                    |
| `src/models/network.h`                  | `tests/models/network_test.cpp`                  |
| `src/models/mnist.h`                    | `tests/models/mnist_test.cpp`                    |
| `src/engine/input/event_bus.h`          | `tests/engine/event_bus_test.cpp`                |
| `src/presenters/pixel_grid_presenter.h` | `tests/presenters/pixel_grid_presenter_test.cpp` |

Shared test utilities live in `tests/test_helpers.h`.

## Conventions

**Name each case after the behaviour, not the function.** The existing cases read
`Test layer update softmax of identical inputs is uniform` rather than
`test_layer_update_5`. A reader scanning the Catch2 output should learn what the
code guarantees without opening the file. Start every case with `Test`.

**One behaviour per case.** Prefer many small cases over one large one. When a case
fails, the name tells you which guarantee broke, and a second failure in a different
case does not hide the first.

**Put setup in the anonymous namespace.** Each file opens `namespace { ... }` after
its includes and ends it before the `TEST_CASE` blocks, so helpers such as `vec()`,
`make_input_layer()`, or `test_network` cannot collide across translation units.

**Reach private and protected state through a subclass.** `models::network` keeps its
training API protected, so `tests/models/network_test.cpp` widens it:

```cpp
class test_network : public models::network {
public:
  void forward(const Eigen::VectorXf &input) {
    set_input(input);
    update();
  }
};
```

Only the members a test actually needs are exposed. This keeps the test suite from
driving design decisions in the production class.

**Mock the boundary, not the subject.** Presenter tests inject a fake view that
records what it was asked to draw, rather than trying to assert on OpenGL output:

```cpp
class recording_view : public views::pixel_grid_view {
public:
  void draw_grid(const image_array &image_data) override {
    recorder_.drawn = image_data;
    recorder_.draw_count++;
  }

private:
  grid_recorder &recorder_;
};
```

**Compare floats with a tolerance.** Use `CHECK_THAT(value, within_abs(target))`
from `tests/test_helpers.h`. `within_abs` wraps Catch2's `WithinAbsMatcher` so that
`float` targets do not need a manual cast, and defaults to an epsilon of `1e-5`.

**Name local constants in `SCREAMING_CASE`.** Values that encode an expected result,
such as the brush intensity under a centered cursor, get a name and a comment saying
where the number comes from:

```cpp
constexpr float BRUSH_CENTER{0.934031f};
constexpr float BRUSH_EDGE{0.262443f};
```

## What is not tested

Anything requiring an OpenGL context is out of reach for unit tests:
`engine::window`, `engine::graphics::renderer`, `engine::graphics::shader`, and
`views::pixel_grid_view_gl`. These need a real display, a shader compile, and a
draw call to observe anything. `views::pixel_grid_view` is an abstract interface
precisely so that `presenters::pixel_grid_presenter` can be tested without one, via
`recording_view`.

`engine::application` wires the window, the scene, and the presenters together and is
likewise untested beyond its indirect use.
