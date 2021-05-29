#include "compute.h"
#include "headers.h"

double my_abs(double n) {
    if (n < 0) n *= -1;
    return n;
}

int my_sign(double n) {
    if (n > 0)
        return 1;
    else if (n < 0)
        return -1;
    else
        return 0;
}

float max(float a, float b) {
    float ret;
    (a > b) ? (ret = a) : (ret = b);
    return ret;
}

float min(float a, float b) {
    float ret;
    (a > b) ? (ret = b) : (ret = a);
    return ret;
}

void multiply_vel(Velocity *vel, float mult) {
    vel->X *= mult;
    vel->Y *= mult;
}

Velocity rotate(Velocity old_vel, double angle) {
    Velocity new_vel;
    new_vel.X = (cos(angle) * old_vel.X) - (sin(angle) * old_vel.Y);
    new_vel.Y = (sin(angle) * old_vel.X) + (cos(angle) * old_vel.Y);
    return new_vel;
}
