mut counter = 0;
increment = fn() mut(counter) { counter = counter + 1; return counter; };
x = increment();
print x;
y = increment();
print y;
// EXPECTED: 1.000000000000000
// EXPECTED: 2.000000000000000
