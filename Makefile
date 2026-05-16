CXX      := g++
CXXFLAGS := -Wall -std=c++23 -MMD -MP
LDFLAGS  := -Llib -lglfw3
INCLUDES := -Iinclude -Isrc

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
else
	CXXFLAGS := $(CXXFLAGS) -g
endif

SRCS := $(shell find src -name "*.cpp")
OBJS := $(SRCS:%.cpp=bin/%.o)
DEPS := $(OBJS:%.o=%.d)

TARGET=bin/main

all: build

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LDFLAGS) -o $(TARGET)

bin/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: build
build: $(TARGET)

.PHONY: run
run: $(TARGET)
	$(TARGET)

.PHONY: clean
clean:
	rm -rf bin

-include $(DEPS)
