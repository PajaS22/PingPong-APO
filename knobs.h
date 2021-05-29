#ifndef KNOBS_H
#define KNOBS_H

#include "headers.h"

typedef struct {
    int8_t rk, gk, bk, rb, gb, bb;
} knobs_data;

/* returns actual knobs position */
knobs_data knobs_value();

/* 
 * sets the knobs memory base adress
 * find the actual knobs position for future relative computations
 */
bool knobs_init();

/* returns the relative position to the position found in the last call of this function */
knobs_data get_rel_knob_value();

#endif
