#!/bin/bash

LD_FLAGS = -lglfw -lGL -ldl

tgv: main.o core.o gl3w.o
	g++ -Iinclude main.o core.o gl3w.o -o tgv $(LD_FLAGS)

gl3w.o: src/gl3w.c
	g++ -c -Iinclude src/gl3w.c 

core.o: src/core.cpp  
	g++ -c -Iinclude src/core.cpp

main.o: src/main.cpp 
	g++ -c -Iinclude src/main.cpp 

.PHONY: clean

clean:
	rm -f *.o
