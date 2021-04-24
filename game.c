// file for handling game objects
// will handle colisions
#include "game.h"

int level;
unsigned short *frame_buff;

void start_game(unsigned short *fb, int lev){
    level = lev;
    frame_buff = fb;
    printf("Selected level %d\n", lev);
    set_display_black(frame_buff);
    countdown(115, 105, 5, frame_buff);
    game_loop();
    set_display_black(frame_buff);
}

void game_loop(){
    Ball ball = (Ball){.pos = (Position){100, 100}, .velocity = (Position){1, -2}, .radius = 10, .color = WHITE};
    Paddle left = (Paddle){.pos = (Position){0, 20}, .color = RED, .height = PADDLE_LENGTH, .width = PADDLE_WIDTH};
    Paddle right = (Paddle){.pos = (Position){DISPLAY_WIDTH-PADDLE_WIDTH, 20}, .color = BLUE, .height = PADDLE_LENGTH, .width = PADDLE_WIDTH};
    draw_ball(ball.pos, ball.radius, ball.color, frame_buff);
    draw_paddle(left.pos, left.color, frame_buff);
    draw_paddle(right.pos, right.color, frame_buff);
    update_display(frame_buff);
    bool goal = false;
    while(!goal){
        Position old = ball.pos;
        int r = move_ball(&ball, &left, &right);
        if(r)
            goal = true;
        update_ball(frame_buff, (Ball){.pos = old, .radius = ball.radius, .color = ball.color}, ball);
        update_display(frame_buff);
        sleep(0.01);
    }
    sleep(1);
} 