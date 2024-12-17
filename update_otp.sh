#!/bin/bash

# Path: Directory where the LLVM source code is located in the "root" path of the project

cd $ROOT/BUILD

make -j4 opt
make -j4 install-opt
