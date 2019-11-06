SHELL = /bin/bash

# Variables used for compiling sources
INC =$(wildcard inc/*.hpp)
SRC =$(wildcard src/*.cpp)
OBJ =$(SRC:src/%.cpp=obj/%.o)
CC = g++-8
LLVMOOPT=$(shell llvm-config --cxxflags)
LLVMLOPT=$(shell llvm-config --ldflags --system-libs --link-static --libs x86codegen)
SIGCOOPT=$(shell pkg-config --cflags sigc++-2.0)
SIGCLOPT=$(shell pkg-config --libs sigc++-2.0)
OOPT = $(LLVMOOPT) -Iinc -std=gnu++17 -g -c -D__ALIOTH_DEBUG__
LOPT = $(LLVMLOPT) -lpthread
TARGET = bin/alioth

# This target is used for test purpose only !
# copy all configuration files to root .
# copy program to root path .
# copy completion file to bash-completion folder .
install: $(TARGET) ./doc/alioth
	sudo cp doc/*.json /usr/lib/alioth/doc/
	sudo cp $(TARGET) /usr/bin/
	sudo cp ./doc/alioth /usr/share/bash-completion/completions/

# link all object files to compiler
$(TARGET):$(OBJ)
	$(CC) $(OBJ) $(LOPT) -o $@

# compile every single source code document to object file
$(OBJ):obj/%.o:src/%.cpp $(INC)
	$(CC) $(OOPT) -o $@ $<

# initial project structure
init:
	if ! [ -e obj ]; then mkdir obj; fi
	if ! [ -e bin ]; then mkdir bin; fi

clean:
	rm -rf obj/*.o bin/*

.PHONY: init clean install