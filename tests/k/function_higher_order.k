apply = fn(f, x) => f(x);
square = fn(x) => x * x;
result = apply(square, 5);
print result;
// EXPECTED: 25.000000000000000
