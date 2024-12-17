#!/bin/bash

# Path: Directory where the LLVM source code is located in the "root" path of the project

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PATH=${SCRIPT_DIR}/INSTALL/bin:$PATH
export ROOT=${SCRIPT_DIR}

echo "New PATH: $PATH"
echo "New ROOT: $ROOT"
