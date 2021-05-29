#ifndef PADDLE_H
#define PADDLE_H

#include "headers.h"

#define PADDLE_WIDTH 10
#define PADDLE_LENGTH 120

typedef struct {
    Position pos;
    int width;
    int height;
    pixel color;
} Paddle;

/* initializes all data for the paddle */
void init_paddle(Paddle *p, Position pos, pixel color);

/* draw paddle in its actual position in the frame buffer */
void draw_paddle(Position position, int paddle_width, int paddle_length, pixel color, pixel *frame_buff);

/*
 * draws the old paddle in the color of the background
 * than draws the new paddle in the color argument in the new position
 */
void update_paddle(Position position, Position old_position, int width, int length, int old_length, pixel color, pixel *frame_buff);

/*
 * changes the y-coordinate of position depending on the move integer
 * controls if the new position is legal (in the display borders)
 */
void move_paddle(Paddle *p, int move);

#endif
