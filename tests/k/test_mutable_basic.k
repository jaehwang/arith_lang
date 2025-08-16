// EXPECTED: Initial value: 42.000000000000000After mutation: 100.000000000000000After expression: 150.000000000000000Complex expression: 7.000000000000000Two variables: 10.000000000000000 and 20.000000000000000
// Basic mutable variable functionality test
mut x = 42.0;
print "Initial value: %.15f", x;

// Change the mutable variable
x = 100.0;
print "After mutation: %.15f", x;

// Use mutable variable in expressions
x = x + 50.0;
print "After expression: %.15f", x;

// Mutable variable with complex expression
mut y = 1 + 2 * 3;
print "Complex expression: %.15f", y;

// Multiple mutable variables
mut a = 10.0;
mut b = 20.0;
print "Two variables: %.15f and %.15f", a, b;
