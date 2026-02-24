#!/bin/bash

LD_FLAGS = -lglfw -lGL -ldl -lpthread

CXXFLAGS = -Iinclude -Iexternal -Iexternal/imgui -Iexternal/imgui/backends

SRC = src/main.cpp src/core.cpp external/gl3w.c \
      external/imgui/imgui.cpp \
      external/imgui/imgui_draw.cpp \
      external/imgui/imgui_tables.cpp \
      external/imgui/imgui_widgets.cpp \
      external/imgui/backends/imgui_impl_glfw.cpp \
      external/imgui/backends/imgui_impl_opengl3.cpp

OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)

tgv: $(OBJ)
	g++ $(OBJ) -o tgv $(LD_FLAGS)

%.o: %.cpp
	g++ -c $(CXXFLAGS) $< -o $@

%.o: %.c
	g++ -c $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f src/*.o external/*.o external/imgui/*.o external/imgui/backends/*.o  tgv
