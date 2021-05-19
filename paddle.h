#ifndef PADDLE_H
#define PADDLE_H

#include "headers.h"

#define PADDLE_WIDTH 10
#define PADDLE_LENGTH 100

typedef struct {
    Position pos;
    int width;
    int height;
    unsigned short color;
} Paddle;

void init_paddle(Paddle *p, Position pos, unsigned short color);
void draw_paddle(Position position, int paddle_width, int paddle_length, unsigned short color, unsigned short *frame_buff);
void update_paddle(Position position, Position old_position, int width, int length, int old_length, unsigned short color, unsigned short *frame_buff);

#endif
