define void @foo(i32 %x, i32 %y) {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %sum = alloca i32, align 4
  store i32 10, i32* %a, align 4
  store i32 20, i32* %b, align 4
  store i32 0, i32* %sum, align 4

  br label %for_cond

for_cond:
  %i = phi i32 [ 0, %entry ], [ %i_next, %for_body ]
  %cmp = icmp slt i32 %i, 10
  br i1 %cmp, label %for_body, label %for_end

for_body:
  %a_val = load i32, i32* %a, align 4
  %b_val = load i32, i32* %b, align 4
  %x_val = add i32 %x, 10
  %y_val = add i32 %y, 20
  %sum_val = add i32 %a_val, %b_val
  %sum_val_2 = add i32 %sum_val, %x_val
  %sum_val_3 = add i32 %sum_val_2, %y_val
  store i32 %sum_val_3, i32* %sum, align 4

  %i_next = add i32 %i, 1
  br label %for_cond

for_end:
  %sum_exit = load i32, i32* %sum, align 4
  call void @print_sum(i32 %sum_exit)
  ret void
}

declare void @print_sum(i32)

define i32 @main() {
entry:
  call void @foo(i32 5, i32 3)
  ret i32 0
}
