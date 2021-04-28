// pingpong ball

#include "ball.h"

#define POW2(x) x*x

void init_ball(Ball *b, Position pos, Velocity velocity, unsigned short color, int radius){
    b->color = color;
    b->radius = radius;
    b->velocity = velocity;
    b->pos = pos;
}

void draw_ball(Position pos, int radius, unsigned short color, unsigned short *frame_buff){
    int first_x;
    for(int y = -radius; y <= radius; ++y){
        first_x = round(sqrt(POW2(radius) - POW2(y)));
        for(int x = -first_x; x <= first_x; ++x){
            draw_pixel(x + pos.X, y + pos.Y, color, frame_buff);
        }
    }
}

void update_ball(Position pos, Position old_pos, int radius, int old_radius, unsigned short color, unsigned short *frame_buff){
    draw_ball(old_pos, old_radius, BACKGROUND_COLOR, frame_buff); // delete old from frame buffer
    draw_ball(pos, radius, color, frame_buff); // draw new ball
}

Position new_position(Ball *ball){
    return (Position){.X = ball->pos.X + ball->velocity.X, .Y = ball->pos.Y + ball->velocity.Y};
}

bool paddle_touch(int ball_y, int paddle_y, int paddle_height){
    return (ball_y >= paddle_y && ball_y <= paddle_y + paddle_height);
}
