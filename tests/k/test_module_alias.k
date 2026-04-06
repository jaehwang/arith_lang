// EXPECTED: 78.539750
import { PI as myPI, square as mySquare } from "math";
radius = 5.0;
area = myPI * mySquare(radius);
print "%.6f\n", area;
