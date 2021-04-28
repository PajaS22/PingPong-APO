#ifndef HEADERS_H
#define HEADERS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

typedef struct{
    float X;
    float Y;
}Position;

typedef struct{
    float X;
    float Y;
}Velocity;

void call_stty(int reset);

#endif