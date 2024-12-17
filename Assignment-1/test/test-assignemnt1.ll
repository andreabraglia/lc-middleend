; Path: TEST/test2-assignment1.ll
; Function to test multiplication by a power of 2 (e.g., x * 8)
define i32 @multiply_by_eight(i32 %x) {
entry:
  %result = mul i32 %x, 8
  ret i32 %result
}

; Function to test addition with zero (e.g., x + 0)
define i32 @add_zero(i32 %x) {
entry:
  %result = add i32 %x, 0
  ret i32 %result
}

; Function to test multiplication by one (e.g., x * 1)
define i32 @multiply_by_one(i32 %x) {
entry:
  %result = mul i32 %x, 1
  ret i32 %result
}

; Function to test advanced strength reduction (e.g., x * 15)
define i32 @multiply_by_fifteen(i32 %x) {
entry:
  %result = mul i32 %x, 15
  ret i32 %result
}

; Function to test division by a power of 2 (e.g., x / 8)
define i32 @divide_by_eight(i32 %x) {
entry:
  %result = sdiv i32 %x, 8
  ret i32 %result
}

; Function to test multiplication by (2^N - 1) (e.g., x * 15)
define i32 @multiply_by_fifteen_alt(i32 %x) {
entry:
  %result = mul i32 %x, 15
  ret i32 %result
}

; Function to test unsigned division by a power of 2 (e.g., x udiv 8)
define i32 @unsigned_divide_by_eight(i32 %x) {
entry:
  %result = udiv i32 %x, 8
  ret i32 %result
}

; Function to test addition with zero when zero is the first operand (e.g., 0 + x)
define i32 @add_zero_first(i32 %x) {
entry:
  %result = add i32 0, %x
  ret i32 %result
}

; Function to test multiplication by one when one is the first operand (e.g., 1 * x)
define i32 @multiply_by_one_first(i32 %x) {
entry:
  %result = mul i32 1, %x
  ret i32 %result
}

; Function to test nested optimizations
; a = x * 1
; b = a + 0
define i32 @nested_optimizations(i32 %x) {
entry:
  %a = mul i32 %x, 1
  %b = add i32 %a, 0
  ret i32 %b
}

; Function to test multi-instruction optimization
; a = b + 1
; c = a - 1
define i32 @multi_instruction_opt(i32 %b) {
entry:
  %a = add i32 %b, 7
  %c = sub i32 %a, 7
  ret i32 %c
}