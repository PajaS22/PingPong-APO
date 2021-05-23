#ifndef PADDLE_H
#define PADDLE_H

#include "headers.h"

#define PADDLE_WIDTH 10
#define PADDLE_LENGTH 120

typedef struct {
    Position pos;
    int width;
    int height;
    ushort color;
} Paddle;

void init_paddle(Paddle *p, Position pos, ushort color);
void draw_paddle(Position position, int paddle_width, int paddle_length, ushort color, ushort *frame_buff);
void update_paddle(Position position, Position old_position, int width, int length, int old_length, ushort color, ushort *frame_buff);
void move_paddle(Paddle *p, int move);

#endif
