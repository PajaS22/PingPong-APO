// file for handling game objects
// will handle colisions
#include "game.h"
#include "LCD_output.h"

int level;

void start_game(unsigned short *frame_buff, int lev){
    level = lev;
    printf("Selected level %d\n", lev);
    set_display_black(frame_buff);
    countdown(115, 105, 5, frame_buff);
    game_loop();
}

void game_loop(){
    if (level == Normal) {
    
    } else if (level >= Hard){
        
    }
} 