SHELL = /bin/bash

# Variables used for compiling sources
INC =$(wildcard inc/*.hpp) inc/lexical.hpp inc/vt.hpp
SRC =$(wildcard src/*.cpp) src/lexical.cpp
TSC =$(wildcard test/*.cpp)
OBJ =$(SRC:src/%.cpp=obj/%.o)
TST =$(TSC:test/%.cpp=bin/test-%)
CC = g++-8
LLVMOOPT=$(shell llvm-config --cxxflags)
LLVMLOPT=$(shell llvm-config --ldflags --system-libs --link-static --libs x86codegen)
OOPT =$(LLVMOOPT) -Iinc -I../utils/inc -std=gnu++17 -g -c -D__ALIOTH_DEBUG__
LOPT =$(LLVMLOPT) -L../utils/arc -lpthread -lutils
TOPT =$(shell llvm-config --cxxflags --ldflags --system-libs --link-static --libs x86codegen) -Iinc -std=gnu++17 -g
TARGET = bin/alioth

# link all object files to compiler
$(TARGET):$(OBJ)
	$(CC) $(OBJ) $(LOPT) -o $@

# compile every single source code document to object file
$(OBJ):obj/%.o:src/%.cpp $(INC)
	$(CC) $(OOPT) -o $@ $<

inc/lexical.hpp: doc/lexical.alex
	alex doc/lexical.alex -l alioth -g lexical.hpp -p inc/

src/lexical.cpp: doc/lexical.alex
	alex doc/lexical.alex -l alioth -g lexical.cpp -p inc/

inc/vt.cpp: doc/lexical.alex
	alex doc/lexical.alex -l alioth -g vt.hpp -p inc/

# test target
test: $(TST)

# compile all test units
$(TST):bin/test-%:test/%.cpp
	$(CC) $< $(TOPT) -o $@

# copy all configuration files to root .
# copy program to root path .
# copy completion file to bash-completion folder .
install: $(TARGET) ./doc/alioth
	sudo cp doc/*.json /usr/lib/alioth/doc/
	sudo cp $(TARGET) /usr/bin/
	sudo cp ./doc/alioth /usr/share/bash-completion/completions/

# initial project structure
init:
	if ! [ -e obj ]; then mkdir obj; fi
	if ! [ -e bin ]; then mkdir bin; fi

clean:
	rm -rf obj/*.o bin/*

.PHONY: init clean install