# Path: TEST/

# Makefile usato per testare il passo di ottimizzazione localopts
BUILD_DIR=../../BUILD/
TEST_FILE=LICM.c
PASS=LICMZ

all: test

build:
	@make -j4 -C $(BUILD_DIR) opt
	@make -j4 -C $(BUILD_DIR) install opt

test_cpp:
	@echo "Running test on $(TEST_FILE) - Optimized: $(patsubst %.c,%,$(TEST_FILE)).optimized.ll\n"
	@clang -O1 -S -emit-llvm $(TEST_FILE) -o "$(patsubst %.c,%,$(TEST_FILE)).ll"
	@opt -passes=mem2reg,licmz $(patsubst %.c,%,$(TEST_FILE)).ll -o "$(patsubst %.c,%,$(TEST_FILE)).optimized.bc"
	@llvm-dis "$(patsubst %.c,%,$(TEST_FILE)).optimized.bc" -o "$(patsubst %.c,%,$(TEST_FILE)).optimized.ll"
	@echo "Optimized file: $(patsubst %.c,%,$(TEST_FILE)).optimized.ll"

test:
	@echo "Running test on $(TEST_FILE) - Optimized: $(patsubst %.ll,%,$(TEST_FILE)).optimized.ll\n"
	@opt -passes=licmz  $(patsubst %.ll,%,$(TEST_FILE)).ll -o "$(patsubst %.ll,%,$(TEST_FILE)).optimized.bc"
	@llvm-dis "$(patsubst %.ll,%,$(TEST_FILE)).optimized.bc" -o "$(patsubst %.ll,%,$(TEST_FILE)).optimized.ll"
	@echo "Optimized file: $(patsubst %.ll,%,$(TEST_FILE)).optimized.ll"