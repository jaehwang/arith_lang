pi_over_4 = 0.0;
denominator = 1.0;
sign = 1.0;
iterations = 0;

while (iterations < 10000) {
    term = sign / denominator;
    pi_over_4 = pi_over_4 + term;
    denominator = denominator + 2.0;
    sign = -1 * sign;
    iterations = iterations + 1;
}

pi_final = pi_over_4 * 4.0;
print pi_final;
