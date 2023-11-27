#!/bin/bash

# Create the output folder
mkdir -p out

# Compile the tests
clang++ -std=c++17 -Wall -Wextra -Werror -lstdc++ -lm -Iinclude/ tests.cpp -o out/tests

# Run the tests
./out/tests