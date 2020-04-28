// RUN: %clang_cc1 -fsyntax-only -fpattern-matching -ast-dump %s | FileCheck %s

void TestInspect(int a, int b) {
  inspect(3) {
    __:;
  }
  // CHECK: InspectStmt 0x{{[^ ]*}} <line:[[@LINE-3]]:3, line:[[@LINE-1]]:3> has_implicit_result_type
  // CHECK-NEXT: IntegerLiteral 0x{{[^ ]*}} <line:[[@LINE-4]]:11> 'int' 3
  // CHECK-NEXT: CompoundStmt 0x{{[^ ]*}} <col:14, line:[[@LINE-3]]:3>
  // CHECK-NEXT: WildcardPatternStmt 0x{{[^ ]*}} <line:[[@LINE-5]]:5, col:8>
  // CHECK-NEXT: NullStmt

  inspect(a) {
    __ if (b>0) :;
  }
  // CHECK: InspectStmt 0x{{[^ ]*}} <line:[[@LINE-3]]:3, line:[[@LINE-1]]:3> has_implicit_result_type
  // CHECK: WildcardPatternStmt 0x{{[^ ]*}} <line:[[@LINE-3]]:5, col:18> has_guard
  // CHECK-NEXT: NullStmt 0x{{[^ ]*}} <col:18>
  // CHECK-NEXT: BinaryOperator 0x{{[^ ]*}} <col:12, col:14> 'bool' '>'

  inspect(3) -> int {
    __:;
  }
  // CHECK: InspectStmt 0x{{[^ ]*}} <line:[[@LINE-3]]:3, line:[[@LINE-1]]:3> has_explicit_result_type
  // CHECK-NEXT: IntegerLiteral 0x{{[^ ]*}} <line:[[@LINE-4]]:11> 'int' 3
  // CHECK-NEXT: CompoundStmt 0x{{[^ ]*}} <col:21, line:[[@LINE-3]]:3>
  // CHECK-NEXT: WildcardPatternStmt 0x{{[^ ]*}} <line:[[@LINE-5]]:5, col:8>
  // CHECK-NEXT: NullStmt
}

