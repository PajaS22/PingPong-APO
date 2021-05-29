#include "paddle.h"
#include "LCD_output.h"

void init_paddle(Paddle *p, Position pos, pixel color) {
    p->color = color;
    p->height = PADDLE_LENGTH;
    p->width = PADDLE_WIDTH;
    p->pos = pos;
}

void draw_paddle(Position position, int paddle_width, int paddle_length, pixel color, pixel *frame_buff) {
    int x = position.X;
    int y = position.Y;
    for (int i = 0; i < paddle_length; ++i) {
        for (int j = 0; j < paddle_width; ++j) {
            draw_pixel(x + j, y + i, color, frame_buff);
        }
    }
}

void update_paddle(Position position, Position old_position, int width, int length, int old_length, pixel color, pixel *frame_buff) {
    draw_paddle(old_position, width, old_length, BACKGROUND_COLOR, frame_buff);
    draw_paddle(position, width, length, color, frame_buff);
}

void move_paddle(Paddle *p, int move) {
    int new_y = p->pos.Y + move;
    if (new_y >= 0 && new_y + p->height < DISPLAY_HEIGHT) { p->pos.Y = new_y; }
}
