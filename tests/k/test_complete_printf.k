// EXPECTED: Simple stringString with
// EXPECTED: newlineQuote: "Hello"Integer: 42
// EXPECTED: Float: 3.142
// EXPECTED: Scientific: 3.141590e+00
// EXPECTED: General: 3.14159
// EXPECTED: Name: ArithLang
// EXPECTED: x=1.500000, y=2.500000, sum=4.000000
// EXPECTED: Progress: % complete
// EXPECTED: 42.500000000000000

// Complete printf functionality test per print.md specification

// Basic string literals
print "Simple string";
print "String with\nnewline";  
print "Quote: \"Hello\"";

// Format strings with arguments
x = 42.7;
print "Integer: %d\n", x;
pi = 3.14159;
print "Float: %.3f\n", pi;
print "Scientific: %e\n", pi;
print "General: %g\n", pi;

// String format specifier  
print "Name: %s\n", "ArithLang";

// Mixed format
a = 1.5;
b = 2.5; 
c = 4.0;
print "x=%f, y=%f, sum=%f\n", a, b, c;

// Literal percent
print "Progress: %% complete\n";

// Backward compatibility (automatic newline)
print 42.5;