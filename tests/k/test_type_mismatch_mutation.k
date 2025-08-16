// Type mismatch on mutation: number -> string should error
// EXPECTED: error: mismatched types

mut a = 1.0;
a = "hello";
