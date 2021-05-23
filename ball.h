#ifndef BALL_H
#define BALL_H

#include "headers.h"
#include "LCD_output.h"
#include "game.h"
#include "paddle.h"

typedef struct {
    Position pos;
    int radius;
    Velocity velocity;
    unsigned short color;
} Ball;

void init_ball(Ball *b, Position pos, Velocity velocity, unsigned short color, int radius);
void draw_ball(Position pos, int radius, unsigned short color, unsigned short *frame_buff);
void update_ball(Position pos, Position old_pos, int radius, int old_radius, unsigned short color, unsigned short *frame_buff);
Position new_position(Ball *ball);
bool paddle_touch(float ball_y, float paddle_y, int paddle_height);
double rebound_angle(float ball_y, float paddle_y, int paddle_height);

#endif
