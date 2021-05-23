#ifndef BONUS_H
#define BONUS_H
#include "ball.h"

#define YELLOW hsv2rgb_lcd(58, 255, 255)
#define TEAL hsv2rgb_lcd(180, 255, 255)
#define NO_HIT -1

#define MID_AREA_LENGTH 200
#define MID_AREA_LEFT ((DISPLAY_WIDTH / 2) - (MID_AREA_LENGTH / 2))

typedef enum { POINTS_BONUS, ENLARGE_BONUS } bonus_type;

typedef struct {
    Ball ball;
    bonus_type bonus_mode;
} Bonus;

// set random position within specified area
void init_bonus(Bonus *b, int radius);
void print_bonuses(Bonus *bonuses, int nbr_bonuses, ushort *frame_buff);
void print_bonus(Bonus *bonus, ushort *frame_buff);
int bonus_hit(Bonus *bonuses, int nbr_bonuses, Ball *ball);
bool bonus_touch(Bonus *bonus, Ball *ball);
Bonus remove_bonus(Bonus *bonuses, int idx, int nbr_bonuses);

#endif