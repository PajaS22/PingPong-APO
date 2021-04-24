#ifndef PADDLE_H
#define PADDLE_H

#include "headers.h"

#define PADDLE_WIDTH 5
#define PADDLE_LENGTH 300

typedef struct {
    Position pos;
    int width;
    int height;
    unsigned short color;
} Paddle;

void draw_paddle(Position position, unsigned short color, unsigned short *frame_buff);

#endif
