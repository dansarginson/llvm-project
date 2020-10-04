// RUN: %clang_cc1 -fsyntax-only -verify -fpattern-matching -Wno-unused-value -Wno-string-compare %s

void noParen() {
  inspect 42 { // expected-error {{expected '(' after 'inspect'}}
    __ => {};
  };
}

void noParenConstExpr() {
  inspect constexpr 42 { // expected-error {{expected '(' after 'constexpr'}}
    __ => {};
  };
}

void noSemiColon() {
  inspect(42) {
    __ => {};
  } // expected-error {{expected ';' after expression}}
}

void trailingReturnTypes() {
  int r = 0;
  r = inspect(42) -> int {
    __ => 3;
  };

  r = inspect(42) -> decltype(1) {
    __ => 3;
  };

  inspect(42) -> void {
    __ => {};
  };

  int x;
  r = inspect(42) -> decltype(x) {
    __ => 3;
  };

  int y;
  inspect(42) -> y { // expected-error {{unknown type name 'y'}}
    __ => {};
  };

  inspect(42) -> { // expected-error {{expected a type}}
    __ => {};
  };

  inspect(42) -> foo { // expected-error {{unknown type name 'foo'}}
    __ => {};
  };
}

// Identifier pattern parsing
int id_pat0() {
  int x = 3;
  return inspect(x) {
    y => y++;
    __ => 3;
  };
}

// Expression pattern parsing
int exp_pat0(int x) {
 return inspect (x) {
    1 => 0;
    2 => 1;
    __ => 42;
 };
}

void exp_pat1(const char *s) {
 inspect (s) {
    "foo" => {}
    "bar" => {}
    __ => {}
 };
}

enum class Color { Red, Green, Blue };

void exp_pat2(Color color) {
  inspect (color) {
    Color::Red => {}
    Color::Green => {}
    Color::Blue => {}
  };
}
