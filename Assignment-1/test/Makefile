# Path: TEST/

# Makefile usato per testare il passo di ottimizzazione localopts
BUILD_DIR=../../BUILD/
TEST_FILE=test-assignment1.ll

all: test

build:
	@make -j4 -C $(BUILD_DIR) opt
	@make -j4 -C $(BUILD_DIR) install opt

test:
	@echo "Running test on $(TEST_FILE) - Optimized: $(patsubst %.ll,%,$(TEST_FILE)).optimized.ll \n"
	@opt -passes=localopts $(TEST_FILE) -o "$(patsubst %.ll,%,$(TEST_FILE)).optimized.bc"
	@llvm-dis "$(patsubst %.ll,%,$(TEST_FILE)).optimized.bc" -o "$(patsubst %.ll,%,$(TEST_FILE)).optimized.ll"
	@echo "Optimized file: $(patsubst %.ll,%,$(TEST_FILE)).optimized.ll"