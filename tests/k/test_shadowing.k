// EXPECTED: Outer x (immutable): 42.000000000000000
// EXPECTED: Shadowed x (immutable): 100.000000000000000
// EXPECTED: Back to outer x: 42.000000000000000
// EXPECTED: Outer y (mutable): 10.000000000000000
// EXPECTED: Mutated y: 20.000000000000000
// EXPECTED: Inner y shadow (mutable): 30.000000000000000
// EXPECTED: Back to outer y: 20.000000000000000
// EXPECTED: Original z (mutable): 1.000000000000000
// EXPECTED: Mutated z: 2.000000000000000
// EXPECTED: Inner z shadow (mutable): 99.000000000000000
// EXPECTED: Back to outer z: 2.000000000000000
// Variable shadowing test
// Tests immutable-by-default with variable shadowing behavior

x = 42.0;
print "Outer x (immutable): %.15f\n", x;

// Shadow immutable x inside an inner block
if (1.0 < 2.0) {
	x = 100.0; // inner-scope immutable shadow
	print "Shadowed x (immutable): %.15f\n", x;
} else {
}
print "Back to outer x: %.15f\n", x;

// Test with mutable variable and inner shadow
mut y = 10.0;
print "Outer y (mutable): %.15f\n", y;

// Mutate the mutable variable
y = 20.0;
print "Mutated y: %.15f\n", y;

// Shadow mutable y with a new inner mutable binding
if (1.0 < 2.0) {
	mut y = 30.0; // inner-scope shadow
	print "Inner y shadow (mutable): %.15f\n", y;
} else {
}
print "Back to outer y: %.15f\n", y;

// Mixed shadowing scenarios
mut z = 1.0;
print "Original z (mutable): %.15f\n", z;

z = 2.0; // mutation
print "Mutated z: %.15f\n", z;

// Shadow z with an inner binding
if (1.0 < 2.0) {
	mut z = 99.0;
	print "Inner z shadow (mutable): %.15f\n", z;
} else {
}
print "Back to outer z: %.15f\n", z;
