#ifndef LED_H
#define LED_H

#include "headers.h"

#define NUM_LEDS_LINE 32
#define ALL_GOALS (NUM_LEDS_LINE / 2)

/* 
 * sets a LED memory base address
 * sets all LEDs to an off mode
 */
bool led_init();

/* lights up one the two RGB LED with color depending on the player that has scored the goal */
void goal_lights(int goal);

/* shows the number of goals on the LED line*/
void led_line(int goals_l, int goals_r);

/*  blicks n times with LED of a given colour (side) ... red or blue */
void blick_n_times(int n, int period, int colour);

/* sets all LEDs on the LED line off */
void led_line_reset();

#endif
