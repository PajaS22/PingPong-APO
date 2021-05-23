// pingpong ball
#include "ball.h"

#define POW2(x) x *x

void init_ball(Ball *b, Position pos, Velocity velocity, ushort color, int radius) {
    b->color = color;
    b->radius = radius;
    b->velocity = velocity;
    b->pos = pos;
}

void draw_ball(Position pos, int radius, ushort color, ushort *frame_buff) {
    int first_x;
    for (int y = -radius; y <= radius; ++y) {
        first_x = round(sqrt(POW2(radius) - POW2(y)));
        for (int x = -first_x; x <= first_x; ++x) {
            draw_pixel(x + pos.X, y + pos.Y, color, frame_buff);
        }
    }
}

void update_ball(Position pos, Position old_pos, int radius, int old_radius, ushort color, ushort *frame_buff) {
    draw_ball(old_pos, old_radius, BACKGROUND_COLOR, frame_buff);  // delete old from frame buffer
    draw_ball(pos, radius, color, frame_buff);                     // draw new ball
}

Position new_position(Ball *ball) {
    return (Position){.X = ball->pos.X + ball->velocity.X, .Y = ball->pos.Y + ball->velocity.Y};
}

bool paddle_touch(float ball_y, float paddle_y, int paddle_height, int radius) {
    return (ball_y + radius >= paddle_y && ball_y - radius <= paddle_y + paddle_height);
}

// returns double between -1 and 1
double rebound_angle(float ball_y, float paddle_y, int paddle_height) {
    float mid = paddle_y + (paddle_height / 2);
    double ret = (2 * (ball_y - mid) / paddle_height);
    if (ret > 1) ret = 1;
    return ret;
}
