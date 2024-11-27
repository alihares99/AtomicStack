#!/bin/bash

echo "running tests"
cmake --build build --target MyTests --config Debug -j 24
cd build
ctest -C Debug --output-on-failure
# ctest -C Debug --verbose

