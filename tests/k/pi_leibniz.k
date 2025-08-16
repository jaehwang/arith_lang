// EXPECTED: 3.141492653590034

mut pi_over_4 = 0.0;
mut denominator = 1.0;
mut sign = 1.0;
mut iterations = 0;

while (iterations < 10000) {
    term = sign / denominator;
    pi_over_4 = pi_over_4 + term;
    denominator = denominator + 2.0;
    sign = -1 * sign;
    iterations = iterations + 1;
}

pi_final = pi_over_4 * 4.0;
print pi_final;
