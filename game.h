#ifndef GAME_H
#define GAME_H

#include "headers.h"
#include "LCD_output.h"
#include "paddle.h"
#include "ball.h"

#define LEFT_BORDER PADDLE_WIDTH
#define RIGHT_BORDER (DISPLAY_WIDTH - PADDLE_WIDTH)
#define UPPER_BORDER 0
#define BOTTOM_BORDER DISPLAY_HEIGHT

enum{Normal, Hard, Extreme};

void start_game();
void game_loop();

#endif
