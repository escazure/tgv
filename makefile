#!/bin/bash

LD_FLAGS = -lglfw -lGL -ldl

tgv: main.o core.o gl3w.o
	g++ -Iinc main.o core.o gl3w.o -o tgv $(LD_FLAGS)

gl3w.o: src/gl3w.c
	g++ -c -Iinc src/gl3w.c 

core.o: src/core.cpp  
	g++ -c -Iinc src/core.cpp

main.o: src/main.cpp 
	g++ -c -Iinc src/main.cpp 

.PHONY: clean

clean:
	rm -f *.o
