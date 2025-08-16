// Immutable reassignment error test
// This should fail with an error when trying to reassign immutable variable

x = 42.0;
print "Initial immutable value: %.15f", x;

// This should cause an error - attempting to reassign immutable variable
x = 100.0;
// EXPECTED: error: Cannot reassign to immutable variable 'x'

print "This line should not be reached: %.15f", x;
