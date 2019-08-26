#ifndef __main_cpp__
#define __main_cpp__

#include "compiler.hpp"

int main( int argc, char **argv ) {
    return alioth::BasicCompiler( argc, argv ).execute();
}

#endif