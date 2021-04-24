// pingpong paddle
#include "paddle.h"
#include "LCD_output.h"

void draw_paddle(Position position, unsigned short color, unsigned short *frame_buff) {
    int x = position.X;
    int y = position.Y;
    for (int i = 0; i < PADDLE_LENGTH; ++i) {
        for (int j = 0; j < PADDLE_WIDTH; ++j) {
            draw_pixel(x + j, y + i, color, frame_buff);
        }
    }
}