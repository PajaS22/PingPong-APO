// pingpong ball

#include "ball.h"

#define POW2(x) x*x

void draw_ball(Position position, int radius, unsigned short color, unsigned short *frame_buff){
    int first_x;
    for(int y = -radius; y <= radius; ++y){
        first_x = round(sqrt(POW2(radius) - POW2(y)));
        for(int x = -first_x; x <= first_x; ++x){
            draw_pixel(x + position.X, y + position.Y, color, frame_buff);
        }
    }
}

// returns 0 if no goal detected
// returns 1 if goal on right occured
// returns -1 if goal on left occured
int move_ball(Ball *ball, Paddle *left, Paddle *right){
    int ret = 0;
    int radius = ball->radius;
    Position new_pos = new_position(ball); // compute next position
    
    if(new_pos.Y + radius >= DISPLAY_HEIGHT) { // bottom wall
        new_pos.Y = 2 * DISPLAY_HEIGHT - new_pos.Y - 2 * radius;
        ball->velocity.Y = -ball->velocity.Y;
    }else if(new_pos.Y - radius <= 0) { // upper wall
        new_pos.Y = -new_pos.Y + 2 * radius;
        ball->velocity.Y = -ball->velocity.Y;
    }else if(new_pos.X - radius <= LEFT_BORDER){ // left wall
        if(paddle_touch(ball->pos.Y, left->pos.Y, left->height)){
            // paddle touches
            new_pos.X = 2 * LEFT_BORDER - new_pos.X + 2 * radius;
            new_pos.X = -new_pos.X;
            ball->velocity.X = -ball->velocity.X;
        }else{
            ret = 1;
        }
    }else if(new_pos.X + radius >= RIGHT_BORDER){ // right wall
        if(paddle_touch(ball->pos.Y, right->pos.Y, right->height)){
            new_pos.X = 2 * RIGHT_BORDER - new_pos.X - 2 * radius;
            ball->velocity.X = -ball->velocity.X;
        }else{
            ret = -1;
        }
    }ball->pos = new_pos;
    return ret;
}

void update_ball(unsigned short *frame_buff, Ball old_ball, Ball new_ball){
    draw_ball(old_ball.pos, old_ball.radius, BLACK, frame_buff); // delete old from frame buffer
    draw_ball(new_ball.pos, new_ball.radius, WHITE, frame_buff); // draw new ball
}

Position new_position(Ball *ball){
    return (Position){.X = ball->pos.X + ball->velocity.X, .Y = ball->pos.Y + ball->velocity.Y};
}

bool paddle_touch(int ball_y, int paddle_y, int paddle_height){
    return (ball_y >= paddle_y && ball_y <= paddle_y + paddle_height);
}
