#ifndef COMPUTE_H
#define COMPUTE_H

/* return an absolute value of a double */
double my_abs(double n);

/* returns signum(n) */
int my_sign(double n);

/* returns the bigger value out of the two */
float max(float a, float b);

/* returns the lower value out of the two */
float min(float a, float b);

/* multiplies both parts of the vector by a float */
void multiply_vel(Velocity *vel, float mult);

/* returns rotated vector by an angle angle */
Velocity rotate(Velocity old_vel, double angle);

#endif
