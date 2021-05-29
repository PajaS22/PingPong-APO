#ifndef BALL_H
#define BALL_H

#include "LCD_output.h"
#include "game.h"
#include "headers.h"
#include "paddle.h"

typedef struct {
    Position pos;
    int radius;
    Velocity velocity;
    pixel color;
} Ball;

/* initializes all data for the ball */
void init_ball(Ball *b, Position pos, Velocity velocity, pixel color, int radius);

/* draw ball in its actual position in the frame buffer */
void draw_ball(Position pos, int radius, pixel color, pixel *frame_buff);

/*
 * draws the old ball in the color of the background
 * than draws the new ball in the color argument in the new position
 */
void update_ball(Position pos, Position old_pos, int radius, int old_radius, pixel color, pixel *frame_buff);

/* returns the position + velocity of the ball */
Position new_position(Ball *ball);

/* controls whether the ball is in given paddle's range on y-axis */
bool paddle_touch(float ball_y, float paddle_y, int paddle_height, int radius);

/*  
 * computes the ball's rebound angle of the paddle's curved surface
 * returns double between -1 and 1
 */
double rebound_angle(float ball_y, float paddle_y, int paddle_height);

#endif
