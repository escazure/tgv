#!/bin/bash

# Compilers
CC = gcc
CXX = g++

# Flags
CFLAGS = -Iexternal
CXXFLAGS = -std=c++17 -Iinclude  
LDFLAGS = -lglfw -lGL -ldl 

# Source and object files
C_SRC = external/gl3w.c
CPP_SRC = core/main.cpp core/fbtg_core.cpp core/fbtg_callbacks.cpp core/input.cpp rendering/fbtg_render.cpp etc/plugin_system.cpp etc/fbtg_gen.cpp external/obj_exporter.cpp

C_OBJS = $(C_SRC:.c=.o)
CPP_OBJS = $(CPP_SRC:.cpp=.o)
OBJS = $(C_OBJS) $(CPP_OBJS)

# Target
TARGET = fbtg

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o fbtg $(LDFLAGS)

# Compile C src
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ src
core/%.o: core/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

rendering/%.o: rendering/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

etc/%.o: etc/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
external/%.o: external/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS)
