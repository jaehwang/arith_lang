// EXPECTED: tests/k/test_nested_error.k:4:16: error: String literal cannot be used in binary operation (left operand)
// EXPECTED: x = 5 + "test" * 2;
// EXPECTED:                ^
x = 5 + "test" * 2;