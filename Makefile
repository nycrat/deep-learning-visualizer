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
OBJS := $(SRCS:%.cpp=$(BUILD)/%.o)
DEPS := $(OBJS:%.o=%.d)

TARGET=$(BIN)/main

all: build

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LDFLAGS) -o $(TARGET)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: build
build: $(TARGET)

.PHONY: run
run: $(TARGET)
	$(TARGET)

.PHONY: lint
lint:
	clang-tidy $(filter-out src/glad.cpp,$(SRCS)) -- $(CXXFLAGS) $(INCLUDES)

.PHONY: clean
clean:
	rm -rf bin build

-include $(DEPS)
