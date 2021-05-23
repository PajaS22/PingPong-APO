// hard play mode
#include "bonus.h"

// random position within specified area
void init_bonus(Bonus *b, int radius) {
    unsigned short colors[] = {
        [POINTS_BONUS] = YELLOW,
        [ENLARGE_BONUS] = TEAL,
    };

    double random_value1;
    double random_value2;
    double random_value3;
    srand ( time (NULL));

    random_value1 = (double)rand() / RAND_MAX; //double in range 0 to 1
    random_value2 = (double)rand() / RAND_MAX; //double in range 0 to 1
    random_value3 = ((double)rand() / RAND_MAX) * 2 - 1; //double in range -1 to 1

    b->bonus_mode = random_value3 >= 0 ? POINTS_BONUS : ENLARGE_BONUS; 
    b->ball.color = colors[b->bonus_mode];
    b->ball.radius = radius;

    b->ball.pos.X = MID_AREA_LEFT + random_value1 * MID_AREA_LENGTH;
    b->ball.pos.Y = radius + random_value2 * (DISPLAY_HEIGHT - radius * 2);
}

// print all active bonuses
void print_bonuses(Bonus *bonuses, int nbr_bonuses, unsigned short *frame_buff) {
    for (int i = 0; i < nbr_bonuses; ++i)
        draw_ball(bonuses[i].ball.pos, bonuses[i].ball.radius, bonuses[i].ball.color, frame_buff);
}

void print_bonus(Bonus *bonus, unsigned short *frame_buff) {
    draw_ball(bonus->ball.pos, bonus->ball.radius, bonus->ball.color, frame_buff);
}

// returns index of a ball that was hit
int bonus_hit(Bonus *bonuses, int nbr_bonuses, Ball *ball){
    for (int i = 0; i < nbr_bonuses; ++i) {
        if (bonus_touch(&(bonuses[i]), ball)) 
            return i;
    } return NO_HIT;
}

bool bonus_touch(Bonus *bonus, Ball *ball){
    float x_dist, y_dist;
    x_dist = bonus->ball.pos.X - ball->pos.X;
    y_dist = bonus->ball.pos.Y - ball->pos.Y;
    float dist = sqrt(pow(x_dist, 2) + pow(y_dist, 2));
    if(dist <= bonus->ball.radius + ball->radius) // hit
        return true;
    return false;
}

Bonus remove_bonus(Bonus *bonuses, int idx, int nbr_bonuses) {
    Bonus ret = bonuses[idx];
    for(int i = idx + 1; i < nbr_bonuses; ++i) {
        bonuses[i-1] = bonuses[i];
    } return ret;
}