# Assignment 1

## Files

- `LocalOpts.cpp`: Contains the implementation of the local optimization pass.
- `LocalOpts.h`: Contains the declaration of the local optimization pass.

## Setup pass

In order to setup the pass, you need to copy `LocalOpts.cpp` to the `SRC/llvm/lib/Transforms/Utils/LocalOpts.cpp` folder and and `LocalOpts.h`  to `SRC/llvm/include/llvm/Transforms/Utils/LocalOpts.h`.
After that, you have to add `MODULE_PASS("localopts", LocalOpts())` to `SRC/llvm/lib/Passes/PassRegistry.def` and import the header file in `SRC/llvm/lib/Passes/PassBuilder.cpp` with `#include "llvm/Transforms/Utils/LocalOpts.h"`. At the end add `LocalOpts.cpp` to the `SRC/llvm/lib/Transforms/Utils/CMakeLists.txt` file.

## Tests

After building the `BUILD` folder with the new pass, in order to run the tests, you need to run the following command:

```bash
cd test
make test # or make
```

If you want to run a test for a specific file, you can run the following command:

```bash
cd test
make test TEST_FILE=<file_name>
```

> [!NOTE]
> If you want to build the `BUILD` folder, you can do it with the make file in the test folder, but before you have to run the setup script.
> The command to build the `BUILD` folder is the following:
>
> ```bash
> make build
> ```
