#ifndef COMPUTE_H
#define COMPUTE_H

double my_abs(double n);
int my_sign(double n);
Velocity rotate(Velocity old_vel, double angle);
float max(float a, float b);
float min(float a, float b);
void multiply_vel(Velocity *vel, float mult);

#endif