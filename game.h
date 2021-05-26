#ifndef GAME_H
#define GAME_H

#include "LCD_output.h"
#include "ball.h"
#include "paddle.h"

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
#define DEBOUNCE_GREEN_KNOB 150000  // 150 ms

#define UPDATE_RATE 150
#define INITIAL_BALL_POSITION ((Position){.X = DISPLAY_WIDTH / 2, .Y = DISPLAY_HEIGHT / 2})
#define INITIAL_BALL_SPEED 0.2
#define INITIAL_BALL_RADIUS 10
#define INITIAL_PADDLE_LEFT ((Position){.X = 0, .Y = DISPLAY_HEIGHT / 2 - PADDLE_LENGTH / 2})
#define INITIAL_PADDLE_RIGHT ((Position){.X = DISPLAY_WIDTH - PADDLE_WIDTH, .Y = DISPLAY_HEIGHT / 2 - PADDLE_LENGTH / 2})
#define COUNTDOWN_X 115
#define COUNTDOWN_Y 105
#define COUNTDOWN_SCALE 5
#define BLICKING_PERIOD 50000
#define PADDLE_SPEED 5
#define BALL_SPEED 1
#define MAX_GOALS ALL_GOALS
#define PI 3.14159265358979323846
#define ANGLE (PI / 5)
#define EASY_ACCELERATION 1.05
#define HARD_ACCELERATION 1.10
#define GAMELOOP_SLEEP 500
#define RESTORE_TIME 5000000  // usec
#define SPAWN_TIME 6000000    // usec
#define BONUS_RADIUS 7
#define MAX_BONUSES_NBR 5
#define ENLARGE_CONST 1.4                // enlarge paddle
#define MAXIMAL_VELOCITY_ANGLE (PI / 3)  // 60 degrees
#define KEYBOARD_PADDLE_CONTROL 3

enum { Normal, Hard };

void start_game();
void game_loop();
void new_round();
Velocity gen_rand_ball_vel();
void enlarge_paddle(Paddle *pad);
void reduce_paddle(Paddle *pad);
void maximal_angle(Velocity *vel);

#endif
