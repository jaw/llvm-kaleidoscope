@.str = private unnamed_addr constant [13 x i8] c"hello world\0A\00"
@.fmt = private unnamed_addr constant [11 x i8] c"Value: %d\0A\00"

declare i32 @puts(i8* nocapture) nounwind
declare i32 @printf(i8* nocapture readonly, ...) nounwind

define i32 @bar(i8* %c, i32 %i) #0 
{
entry:
  %call = tail call i32 (i8*, ...)* @printf(i8* %c, i32 %i)
  ret i32 %call
}

define i32 @main() {   ; i32()*
  %cast210 = getelementptr [13 x i8]* @.str, i64 0, i64 0
  %cast211 = getelementptr [11 x i8]* @.fmt, i64 0, i64 0

  call i32 @puts(i8* %cast210)
  call i32 @bar(i8* %cast211, i32 67)
  ret i32 0
}

!0 = !{i32 42, null, !"string"}
!foo = !{!0}

