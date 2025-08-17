// EXPECTED: tests/k/test_paren_error.k:4:8: error: String literal cannot be used in binary operation (right operand)
// EXPECTED: x = (3 + "hello") * 2;
// EXPECTED:        ^
x = (3 + "hello") * 2;