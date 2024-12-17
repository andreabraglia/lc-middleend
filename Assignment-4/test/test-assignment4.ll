; ModuleID = 'LoopFusionTest'
source_filename = "LoopFusionTest"

define void @test_function() {
entry:
  ; Variabili di loop
  %arr = alloca [10 x i32], align 4

  ; Loop 1: inizializza l'array
  br label %loop1_header

loop1_header: ; Header del primo loop
  %i = phi i32 [ 0, %entry ], [ %i_next, %loop1_latch ]
  %cmp1 = icmp slt i32 %i, 10
  br i1 %cmp1, label %loop1_body, label %loop1_exit

loop1_body: ; Corpo del primo loop
  %idx1 = getelementptr [10 x i32], [10 x i32]* %arr, i32 0, i32 %i
  store i32 0, i32* %idx1
  br label %loop1_latch

loop1_latch: ; Latch del primo loop
  %i_next = add i32 %i, 1
  br label %loop1_header

loop1_exit: ; Uscita dal primo loop
  br label %loop2_header

loop2_header: ; Header del secondo loop
  %j = phi i32 [ 0, %loop1_exit ], [ %j_next, %loop2_latch ]
  %cmp2 = icmp slt i32 %j, 10
  br i1 %cmp2, label %loop2_body, label %loop2_exit

loop2_body: ; Corpo del secondo loop
  %idx2 = getelementptr [10 x i32], [10 x i32]* %arr, i32 0, i32 %j
  %val = load i32, i32* %idx2
  %updated_val = add i32 %val, 1
  store i32 %updated_val, i32* %idx2
  br label %loop2_latch

loop2_latch: ; Latch del secondo loop
  %j_next = add i32 %j, 1
  br label %loop2_header

loop2_exit: ; Uscita dal secondo loop
  ret void
}
