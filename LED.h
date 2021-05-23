#ifndef LED_H
#define LED_H

#include "headers.h"

#define NUM_LEDS_LINE 32
#define ALL_GOALS (NUM_LEDS_LINE / 2)

bool led_init();
void goal_lights(int goal);
void led_line(int goals_l, int goals_r);
void blick_n_times(int n, int period, int colour);
void led_line_reset();

#endif
