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

void returnTypeDeduction() {
  inspect(42) {
    42 => 42;
    43 => 43;
    __ => !{}; // exclude last from type deduction
  };

  inspect(42) {
    42 => 42;
    __ => !{}; // exclude middle from type deduction
    43 => 43;
  };

  inspect(42) {
    __ => !{}; // exclude first from type deduction
    42 => 42;
    43 => 43;
  };

  inspect(42) {
    __ => !{}; // exclude only pattern
  };

  inspect(42) {
    42 => 42;
    __ => !throw "whoops"; // expected-error {{expected '{' after '!'}}
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

// FIXME: should we support scoped enums?
enum Color { Red, Green, Blue };

void exp_pat2(Color color) {
  inspect (color) {
    Color::Red => {}
    Color::Green => {}
    Color::Blue => {}
  };
}

void exp_pat3(int x) {
  inspect (x) {
    SomeEnum::Red => {} // expected-error {{use of undeclared identifier 'SomeEnum'}}
                        // expected-error@-1 {{expected constant-expression}}
    case RED => {} // expected-error {{use of undeclared identifier 'RED'}}
                   // expected-error@-1 {{expected constant-expression}}
  };
}

void exp_case0(Color color) {
  inspect (color) {
    case Color::Red => {}
    case Color::Green => {}
    case Color::Blue => {}
  };
}

void exp_case1(Color c) {
  inspect (c) {
    case Red => {}
    case Green => {}
    case Blue => {}
  };
}

int exp_case2(int x) {
  int y;
  y = inspect(x) -> int {
    case 0 => 1;
    case 1 => 2;
  };
  return y;
}

void exp_case3(const char *s) {
 inspect (s) {
    case "foo" => {}
    case "bar" => {}
  };
}

void bad_equal_arrow(int x) {
  const int h = 42;
  inspect (x) {
    case h = {} // expected-error {{expected '=>' after constant-expression}}
    y if (h != 0) = > {}; // expected-error {{expected '=>' after if}}
  };
}