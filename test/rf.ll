; ModuleID = 'mod'
source_filename = "mod"

@gv2 = constant i8 24

define void @test() {
  %1 = alloca i8
  %2 = load i8, i8* @gv1
  store i8 %2, i8* %1
  ret void
}