// handle knobs input
#include "knobs.h"
#include "mzapo_regs.h"

#define HALF_KNOB 127

static unsigned char *mem_base;

static int8_t rk, gk, bk; // last positions of knobs

short circle_value(unsigned short val);

bool knobs_init() {
    bool ret = true;
    mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (mem_base == NULL) {
        fprintf(stderr, "ERROR: Physical address could not be allocated!\n");
        ret = false;
    }   
    // set starting values of knobs
    knobs_data kd = knobs_value();
    rk = kd.rk;
    gk = kd.gk;
    bk = kd.bk;
    return ret;
}

knobs_data get_rel_knob_value() {
    knobs_data actual_kd = knobs_value();
    knobs_data ret;
    ret.rk = circle_value(actual_kd.rk - rk);
    ret.gk = circle_value(actual_kd.gk - gk);
    ret.bk = circle_value(actual_kd.bk - bk);

    ret.rb = actual_kd.rb;
    ret.gb = actual_kd.gb;
    ret.bb = actual_kd.bb;

    rk = actual_kd.rk;
    gk = actual_kd.gk;
    bk = actual_kd.bk; 
    
    return ret;
}

short circle_value(unsigned short val){
    if (val > HALF_KNOB) 
        val = val - 255 - 1;    
    return -val;
}

// returns actual knobs position
knobs_data knobs_value() {
    int32_t rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    knobs_data ret = {
        .rk = (rgb_knobs_value >> 16) & 0xFF, // red knob position
        .gk = (rgb_knobs_value >>  8) & 0xFF, // green knob position
        .bk = (rgb_knobs_value >>  0) & 0xFF, // blue knob position
        .rb = (rgb_knobs_value >> 26) & 1,    // red button
        .gb = (rgb_knobs_value >> 25) & 1,    // green button
        .bb = (rgb_knobs_value >> 24) & 1     // blue button
    };
    return ret;
}
