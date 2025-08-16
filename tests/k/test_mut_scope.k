// EXPECTED: Global mutable x: 10.000000000000000
// EXPECTED: In if block, x: 10.000000000000000
// EXPECTED: After mutation in if, x: 20.000000000000000
// EXPECTED: Inner scope mutable y: 100.000000000000000
// EXPECTED: After mutation, y: 200.000000000000000
// EXPECTED: Back in global scope, x: 20.000000000000000
// EXPECTED: Loop iteration, counter: 0.000000000000000
// EXPECTED: Loop temp (mutable): 10.000000000000000
// EXPECTED: After temp mutation: 15.000000000000000
// EXPECTED: Loop iteration, counter: 1.000000000000000
// EXPECTED: Loop temp (mutable): 20.000000000000000
// EXPECTED: After temp mutation: 25.000000000000000
// EXPECTED: Loop iteration, counter: 2.000000000000000
// EXPECTED: Loop temp (mutable): 30.000000000000000
// EXPECTED: After temp mutation: 35.000000000000000
// EXPECTED: Final counter: 3.000000000000000
// EXPECTED: Nested mutation result: 3.000000000000000
// EXPECTED: Final global_var: 3.000000000000000
// Scoped mutability test
// Tests mutability behavior with nested scopes (if/while blocks)

mut x = 10.0;
print "Global mutable x: %.15f\n", x;

// Test mutability in if block
if (x > 5.0) {
    print "In if block, x: %.15f\n", x;
    x = 20.0; // Should work - mutating outer mutable variable
    print "After mutation in if, x: %.15f\n", x;
    
    // Create new variable in inner scope
    mut y = 100.0;
    print "Inner scope mutable y: %.15f\n", y;
    y = 200.0; // Should work - mutating inner mutable variable
    print "After mutation, y: %.15f\n", y;
} else {
    print "This should not execute\n";
}

print "Back in global scope, x: %.15f\n", x;

// Test mutability in while block
mut counter = 0.0;
while (counter < 3.0) {
    print "Loop iteration, counter: %.15f\n", counter;
    counter = counter + 1.0; // Should work - mutating mutable counter
    
    // Shadow with immutable in loop scope
    mut temp = counter * 10.0;
    print "Loop temp (mutable): %.15f\n", temp;
    temp = temp + 5.0; // Should work
    print "After temp mutation: %.15f\n", temp;
}

print "Final counter: %.15f\n", counter;

// Test cross-scope mutation
mut global_var = 1.0;
if (global_var > 0.0) {
    global_var = 2.0; // Should work - outer scope mutable
    if (global_var > 1.0) {
        global_var = 3.0; // Should work - outer scope mutable from nested block
    print "Nested mutation result: %.15f\n", global_var;
    } else {
    print "Should not reach here\n";
    }
} else {
    print "Should not reach here\n";
}

print "Final global_var: %.15f\n", global_var;
