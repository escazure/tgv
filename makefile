#!/bin/bash

# Libraries to link
LD_FLAGS = -lglfw -lGL -ldl -lpthread

# Compiler flags (includes ImGui)
CXXFLAGS = -Iinclude -Iexternal -Iexternal/imgui -Iexternal/imgui/backends

# Sources
SRC = src/main.cpp src/core.cpp external/gl3w.c \
      external/imgui/imgui.cpp \
      external/imgui/imgui_draw.cpp \
      external/imgui/imgui_tables.cpp \
      external/imgui/imgui_widgets.cpp \
      external/imgui/backends/imgui_impl_glfw.cpp \
      external/imgui/backends/imgui_impl_opengl3.cpp

# Object files (replace .cpp/.c with .o)
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)

# Target
tgv: $(OBJ)
	g++ $(OBJ) -o tgv $(LD_FLAGS)

# Compile .cpp files
%.o: %.cpp
	g++ -c $(CXXFLAGS) $< -o $@

# Compile .c files
%.o: %.c
	g++ -c $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o tgv
