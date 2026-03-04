factorial = fn(n) {
    mut result = 0;
    if (n == 0) { result = 1; } else { result = n * factorial(n - 1); }
    return result;
};
result = factorial(5);
print result;
// EXPECTED: 120.000000000000000
