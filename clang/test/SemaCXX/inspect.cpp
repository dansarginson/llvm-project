// RUN: %clang_cc1 -fsyntax-only -verify -fpattern-matching -Wno-unused-value -Wno-string-compare %s

void a() {
  inspect(42) {
    __ => {};
  };
  inspect(b) { // expected-error {{use of undeclared identifier 'b'}}
    __ if (true) => {};
  };
}

void b(int x) {
  inspect(x) -> void {
    __ => 3; // expected-error {{cannot initialize statement expression result of type 'void' with an rvalue of type 'int'}}
  };
}

int c(int x) {
  int w = inspect(x) -> int {
    __ => 2.3; // expected-warning {{implicit conversion from 'double' to 'int' changes value from 2.3 to 2}}
  };
  return w;
}

int bar() { return 4; }

void d(int x) {
  inspect(x) -> void {
    __ => (void)bar();
  };
}

void e(int x) {
  inspect(x) -> void {
    __ => { bar(); };
  };
}

void f(int x) {
  inspect(x) -> int {
    __ => { bar(); }; // expected-error {{resulting expression type 'void' must match trailing result type 'int'}}
  };
}

void baz() { }
void f1(int x) {
  inspect(x) -> int {
    42 => 42;
    __ => !{ baz(); }; // ok because not participating in type deduction
  };
}

void f2(int x) {
  inspect(x) -> int {
    __ => !{ baz(); };  // ok because not participating in type deduction
    42 => 42;
  };
}

void g(int x) {
  inspect(x) -> void {
    1 => (void)3;
    2 =>;
    __ => { bar(); };
  };
}

void h(int x) {
  inspect(x) {
    0 =>;
    __ => (void)3;
  };
}

int i() {
  int x = 3;
  int y = 4;
  inspect (y) {
    x => {} // x is defined within pattern scope, inspect should return 4.
  };
}

void pat_exp_literals(char *s, char c, bool b) {
  inspect(s) {
    case "foo" => {}
  };
  inspect(c) {
    case 'c' => {}
  };
  inspect(b) {
    case true => {}
  };
}

void pat_exp_nodirect_literals(char *s, char c, bool b) {
  const char *foo = "foo";
  inspect(s) {
    // FIXME: should be accepted? See raw_string_tests below for more.
    case foo => {} // expected-error {{pattern is not a constant expression}}
                   // expected-note@-1 {{read of non-constexpr variable 'foo' is not allowed in a constant expression}}
                   // expected-note@-5 {{declared here}}
  };

  const char ch = 'c';
  inspect(c) {
    case ch => {}
  };

  const bool boo = true;
  inspect(b) {
    case boo => {}
  };
}

const int z = int();
constexpr int Sum(int a = 0, const int &b = 0, const int *c = &z, char d = 0) {
  return a + b + *c + d;
}

void integral_constants(int x) { // expected-note {{declared here}}
  const int h = 42;
  struct Y { int a; int b; };
  const Y y = {2, 3};
  Y y2 = {4+x, 3+x}; // expected-note {{declared here}}
  inspect (x) {
    case h => {}
    case y.a => {}
    case y2.b => {} // expected-error {{expression is not an integral constant expression}}
                    // expected-note@-1 {{read of non-constexpr variable 'y2' is not allowed in a constant expression}}
    case ++x => {} // expected-error {{expression is not an integral constant expression}}
                   // expected-note@-1 {{a constant expression cannot modify an object that is visible outside that expression}}
    case 0 => {}
    case Sum(3) => {}
    case Sum(x) => {} //  expected-error {{expression is not an integral constant expression}}
                      //  expected-note@-1 {{read of non-const variable 'x' is not allowed in a constant expression}}
  };
}

struct A {
  static constexpr const char *s = "s";
};
const char *gs = "gs"; // expected-note {{declared here}}
void raw_string_tests(const char *x) {
  static const char *in0 = "in0"; // expected-note {{declared here}}
  const char *in1 = "in1"; // expected-note {{declared here}}
  constexpr const char *in2 = "in2";

  inspect(x) {
    case gs => {}; // expected-error {{pattern is not a constant expression}}
                   // expected-note@-1 {{read of non-constexpr variable 'gs' is not allowed in a constant expression}}
    case in0 => {}; // expected-error {{pattern is not a constant expression}}
                    // expected-note@-1 {{read of non-constexpr variable 'in0' is not allowed in a constant expression}}
    case A::s => {};
    case in1 => {}; // expected-error {{pattern is not a constant expression}}
                    // expected-note@-1 {{read of non-constexpr variable 'in1' is not allowed in a constant expression}}
    case in2 => {};
  };
}

void int_struct(int x) {
  struct s {
    int a;
    int b;
    bool operator==(const s &other) const
    {
      return a != other.a;
    }
  };
  s s1{3,4};
  constexpr s s2{4,5};
  s s3{4,5}; // expected-note {{declared here}}
  const s s4{4,5};
  inspect (s1) {
    case s2 => {};
    case s3 => {}; // expected-error {{pattern is not a constant expression}}
                   // expected-note@-1 {{read of non-constexpr variable 's3' is not allowed in a constant expression}}
    case s4 => {}; // expected-error {{pattern is not a constant expression}}
                   // expected-note@-1 {{read of non-constexpr variable 's4' is not allowed in a constant expression}}
  };
}