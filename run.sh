#!/bin/bash

# Create the output folder
mkdir -p out

# Compile the C++ code
clang++ -std=c++17 -lstdc++ -lm -Iinclude/ project.cpp -Wall -Wextra -Werror -O3 -ltbb -o out/project

# Run the compiled program
./out/project