CXX      := g++
CXXFLAGS := -Wall -std=c++23 -MMD -MP
LDFLAGS  := -Llib -lglfw3
INCLUDES := -Isrc -isystem include

OS := $(shell uname)

# Specific flags for MacOS
ifeq ($(OS),Darwin)
	CXX := clang++
	CXXFLAGS := $(CXXFLAGS) -x c++
	LDFLAGS := $(LDFLAGS) -framework Cocoa -framework OpenGL -framework IOKit -framework QuartzCore
endif

# Check for BUILD=release, default is debug
ifeq ($(BUILD),release)
	CXXFLAGS := $(CXXFLAGS) -O3 -march=native
	BUILD    := build/release
	BIN      := bin/release
else
	CXXFLAGS := $(CXXFLAGS) -g
	BUILD    := build/debug
	BIN      := bin/debug
endif

SRCS := $(shell find src -name "*.cpp")
HRDS := $(shell find src -name "*.h")
OBJS := $(SRCS:%.cpp=$(BUILD)/%.o)
DEPS := $(OBJS:%.o=%.d)

TARGET := $(BIN)/main

TEST_SRCS := $(shell find tests -name "*.cpp")
TEST_HRDS := $(shell find tests -name "*.h")
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD)/%.o)
TEST_DEPS := $(TEST_OBJS:%.o=%.d)

TEST_TARGET := $(BIN)/test

all: build

$(TARGET): $(OBJS)
$(TEST_TARGET): $(TEST_OBJS) $(filter-out $(BUILD)/src/main.o,$(OBJS))

$(TARGET) $(TEST_TARGET):
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LDFLAGS) -o $@

$(BUILD)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -Itests -c $< -o $@

.PHONY: build
build: $(TARGET)

.PHONY: run
run: $(TARGET)
	$(TARGET)

.PHONY: test
test: $(TEST_TARGET)
	$(TEST_TARGET)

.PHONY: lint
lint:
	clang-tidy $(filter-out src/glad.cpp,$(SRCS)) -- $(CXXFLAGS) $(INCLUDES)

.PHONY: format
format:
	clang-format $(SRCS) $(HRDS) -i
	clang-format $(filter-out tests/catch_amalgamated.cpp,$(TEST_SRCS)) $(filter-out tests/catch_amalgamated.h,$(TEST_HRDS)) -i

.PHONY: clean
clean:
	rm -rf bin build

-include $(DEPS)
-include $(TEST_DEPS)
