SHELL = /bin/bash

# Variables used for compiling sources
INC = $(wildcard inc/*.hpp)
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:src/%.cpp=obj/%.o)
CC = g++-8
LLVMOOPT = $(shell llvm-config --cxxflags)
LLVMLOPT = $(shell llvm-config --ldflags --system-libs --link-static --libs x86codegen)
OOPT = $(LLVMOOPT) -Iinc -std=gnu++17 -g -c -D__ALIOTH_DEBUG__
LOPT = $(LLVMLOPT)
TARGET = bin/alioth

# link all object files to compiler
$(TARGET):$(OBJ)
	$(CC) $(LOPT) -o $@ $(OBJ)

# compile every single source code document to object file
$(OBJ):obj/%.o:src/%.cpp $(INC)
	$(CC) $(OOPT) -o $@ $<

# initial project structure
init:
	if ! [ -e obj ]; then mkdir obj; fi
	if ! [ -e bin ]; then mkdir bin; fi

clean:
	rm -rf obj/*.o bin/*

install:
	sudo cp doc/*.json /usr/lib/alioth/doc/

.PHONY: init clean build install