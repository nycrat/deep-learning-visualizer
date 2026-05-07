CXX=g++
CXXFLAGS=-Wall -g -std=c++23
LDFLAGS=-Llib -lglfw3
INCLUDES=-Iinclude

SRCS=src/*.cpp src/glad.c
HEADERS=src/*.h

TARGET=bin/main

OS=$(shell uname)

# Specific flags for MacOS
ifeq ($(OS),Darwin)
		LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit -framework QuartzCore
		CXX=clang++
		CXXFLAGS += -x c++
endif

all: build

$(TARGET): $(SRCS) $(HEADERS)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRCS) $(INCLUDES) $(LDFLAGS) -o $(TARGET)

.PHONY: build
build: $(TARGET)

.PHONY: run
run: $(TARGET)
	$(TARGET)

.PHONY: clean
clean:
	rm -rf bin
