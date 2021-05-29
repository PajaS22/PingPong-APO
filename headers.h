#ifndef HEADERS_H
#define HEADERS_H

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    float X;
    float Y;
} Position;

typedef struct {
    float X;
    float Y;
} Velocity;

typedef uint16_t pixel;

/* sets terminal to raw mode */
void call_stty(int reset);

#endif
