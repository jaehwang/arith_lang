// EXPECTED: 120.000000000000000
mut n = 5;
mut result = 1;
while (n > 0) {
    result = result * n;
    n = n - 1;
}
print result;
