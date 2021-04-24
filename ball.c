// pingpong ball
#include "ball.h"
#include "LCD_output.h"
#include <math.h>

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

