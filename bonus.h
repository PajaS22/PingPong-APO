#ifndef BONUS_H
#define BONUS_H
#include "ball.h"

// BONUS BALLS COLORS
#define YELLOW hsv2rgb_lcd(58, 255, 255)
#define TEAL hsv2rgb_lcd(180, 255, 255)

#define NO_HIT -1 // the bonus ball has not been hit by the ping pong ball

// THE AREA ON THE SCREEN WHERE THE BONUS BALLS CAN OCCURE
#define MID_AREA_LENGTH 200
#define MID_AREA_LEFT ((DISPLAY_WIDTH / 2) - (MID_AREA_LENGTH / 2))

typedef enum { POINTS_BONUS, ENLARGE_BONUS } bonus_type;

typedef struct {
    Ball ball;
    bonus_type bonus_mode;
} Bonus;

/* sets random position within the specified area where the bonus ball can occure */
void init_bonus(Bonus *b, int radius);

/* draws all active bonuses in the frame buffer */
void print_bonuses(Bonus *bonuses, int nbr_bonuses, pixel *frame_buff);

/* prints bonus in the frame buffer */
void print_bonus(Bonus *bonus, pixel *frame_buff);

/* returns the index of ball that was hit <0; MAX_BONUSES_NBR) */
int bonus_hit(Bonus *bonuses, int nbr_bonuses, Ball *ball);

/* controls whether the game ball has touched the bonus ball */
bool bonus_touch(Bonus *bonus, Ball *ball);

/* removes the bonus ball from the bonuses array and shifts all the remaining bonuses after a hit */
Bonus remove_bonus(Bonus *bonuses, int idx, int nbr_bonuses);

#endif
