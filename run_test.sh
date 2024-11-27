#!/bin/bash

echo "running tests"
cmake --build build --target MyTests --config Release -j 24
cd build
ctest -C Release --output-on-failure 
# ctest -C Release --verbose
