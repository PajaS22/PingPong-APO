#ifndef BALL_H
#define BALL_H

#include "headers.h"
#include "LCD_output.h"
#include "game.h"
#include "paddle.h"

typedef struct {
    Position pos;
    int radius;
    Position velocity;
    unsigned short color;
} Ball;

void draw_ball(Position position, int radius, unsigned short color, unsigned short *frame_buff);
int move_ball(Ball *ball, Paddle *left, Paddle *right);
void update_ball(unsigned short *frame_buff, Ball old_ball, Ball new_ball);
Position new_position(Ball *ball);
bool paddle_touch(int ball_y, int paddle_y, int paddle_height);

#endif
