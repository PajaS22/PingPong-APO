#ifndef KNOBS_H
#define KNOBS_H
#include "headers.h"

typedef struct {
    uint8_t rk, gk, bk, rb, gb, bb;
} knobs_data;

// returns actual knobs position
knobs_data knobs_value();
bool knobs_init();

#endif
