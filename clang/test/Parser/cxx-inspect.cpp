// RUN: %clang_cc1 -fsyntax-only -verify -fpattern-matching %s

void NoParen() {
  inspect 42 { // expected-error {{expected '(' after 'inspect'}}
    __:;
  }
}

void NoParenConstExpr() {
  inspect constexpr 42 { // expected-error {{expected '(' after 'constexpr'}}
    __:;
  }
}

void BasicWorkingInspects() {
  inspect(42) {}

  inspect constexpr(42) {}
}

void trailingReturnTypes() {
  inspect(42) -> int {
    __:;
  }

  inspect(42) -> decltype(1) {
    __:;
  }

  inspect(42) -> void {
    __:;
  }

  int x;
  inspect(42) -> decltype(x) {
    __:;
  }

  int y;
  inspect(42) -> y { // expected-error {{unknown type name 'y'}}
    __:;
  }

  inspect(42) -> { // expected-error {{expected a type}}
    __:;
  }

  inspect(42) -> foo { // expected-error {{unknown type name 'foo'}}
    __:;
  }
}
