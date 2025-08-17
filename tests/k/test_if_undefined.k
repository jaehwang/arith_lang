// EXPECTED: tests/k/test_if_undefined.k:4:5: error: cannot find value 'x' in this scope
// EXPECTED: if (x > 5) { print x; } else { print 0; }
// EXPECTED:     ^
if (x > 5) { print x; } else { print 0; }