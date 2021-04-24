// file for handling game objects
// will handle colisions
#include "game.h"
#include "LCD_output.h"
#include "ball.h"
#include "paddle.h"

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
    draw_ball((Position){.X = 100, .Y = 100}, 10, WHITE, frame_buff);
    draw_paddle((Position){.X = 0, .Y = 20}, RED, frame_buff);
    draw_paddle((Position){.X = DISPLAY_WIDTH-PADDLE_WIDTH, .Y = 20}, BLUE, frame_buff);
    update_display(frame_buff);
    sleep(5);
} 