# Lanai Interpreter

Lanai is a processor architecture developed by google.
The reason it is used for TrueBit is because it is a very
simplistic architecture (it is easy to implement an interpreter
in Solidity and it does not have complex memory access patterns)
and there is an actively maintained compiler from C/C++/Rust
to this architecture (llvm).

## How to run the example

To run the example, you have to compile the C code into a
Lanai binary. This will also compile clang for lanai in a docker container
because it is still an experimental backend:

    cd llvm
    ./build.sh

This will generate ``example.o`` (the binary) and ``example.s``
(the lanai assembly for reference).

Now compile the interpreter:

    cd ../interpreter
    mkdir build && cd build
    cmake .. && make

Now you can run the binary file in the interpreter:

    ./lanai-int ../../llvm/example.o

It will tell you the result of the computation and the number
of steps it took. You can set a verbosity flag in ``main.cpp``
to get more output.

Note that the number of steps is quite large because Lanai does not
have multiplication opcodes, but that is mostly irrelevant for
Truebit because of its logarithmic scalability - it is possible to
verify 1000000 computational steps in about five rounds and
for 100000000 steps it takes eight rounds. 