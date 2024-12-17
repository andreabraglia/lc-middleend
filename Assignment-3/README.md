# Assignment 1

## Files

- `LICMZ.cpp`: Contains the implementation of the local optimization pass.
- `LICMZ.h`: Contains the declaration of the local optimization pass.

## Setup pass

In order to setup the pass, you need to copy `LICMZ.cpp` to the `SRC/llvm/lib/Transforms/Utils/LICMZ.cpp` folder and and `LICMZ.h`  to `SRC/llvm/include/llvm/Transforms/Utils/LICMZ.h`.
After that, you have to add `LOOP_PASS("licmz", LICMZ())` to `SRC/llvm/lib/Passes/PassRegistry.def` and import the header file in `SRC/llvm/lib/Passes/PassBuilder.cpp` with `#include "llvm/Transforms/Utils/LICMZ.h"`. At the end add `LICMZ.cpp` to the `SRC/llvm/lib/Transforms/Utils/CMakeLists.txt` file.

## Tests

After building the `BUILD` folder with the new pass, in order to run the tests, you need to run the following command:

```bash
cd test
make test # or make
```

If you want to run a test for a specific file, you can run the following command:

```bash
cd test
make test TEST_FILE=<file_name> # or make test_cpp TEST_FILE=<file_name> for C++ files
```

> [!NOTE]
> If you want to build the `BUILD` folder, you can do it with the make file in the test folder, but before you have to run the setup script.
> The command to build the `BUILD` folder is the following:
>
> ```bash
> make build
> ```
