#ifndef GAME_H
#define GAME_H

#include "LCD_output.h"
#include "paddle.h"
#include "ball.h"

#define NO_GOAL 0
#define LEFT_GOAL -1
#define RIGHT_GOAL 1
#define LEFT_PLAYER -1
#define RIGHT_PLAYER 1
#define DRAW 2

#define LEFT_BORDER PADDLE_WIDTH
#define RIGHT_BORDER (DISPLAY_WIDTH - PADDLE_WIDTH)
#define UPPER_BORDER 0
#define BOTTOM_BORDER DISPLAY_HEIGHT
#define DEBOUNCE_GREEN_KNOB 300000

enum { Normal, Hard };

void start_game();
void game_loop();
void new_round();
Velocity gen_rand_ball_vel();
void multiply_vel(Velocity *vel, float mult);
float max(float a, float b);
float min(float a, float b);
void enlarge_paddle(Paddle *pad);
void reduce_paddle(Paddle *pad);
Velocity rotate(Velocity old_vel, double angle);
void maximal_angle(Velocity *vel);
double my_abs(double n);
int my_sign(double n);

#endif
