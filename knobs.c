// mzapo red & green & blue knobs
#include "knobs.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#define HALF_KNOB 127
#define MOVE_KNOB_GREEN 6
#define MOVE_KNOB_RED_BLUE 0

static unsigned char *mem_base;

static int8_t rk, gk, bk;  // last positions of knobs

short circle_value(ushort val);

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

    (abs(ret.rk) > MOVE_KNOB_RED_BLUE) ? (rk = actual_kd.rk) : (ret.rk = 0);
    (abs(ret.gk) > MOVE_KNOB_GREEN) ? (gk = actual_kd.gk) : (ret.gk = 0);
    (abs(ret.bk) > MOVE_KNOB_RED_BLUE) ? (bk = actual_kd.bk) : (ret.bk = 0);

    ret.rb = actual_kd.rb;
    ret.gb = actual_kd.gb;
    ret.bb = actual_kd.bb;

    return ret;
}

short circle_value(ushort val) {
    if (val > HALF_KNOB) val = val - 255 - 1;
    return -val;
}

// returns actual knobs position
knobs_data knobs_value() {
    int32_t rgb_knobs_value = *(volatile uint32_t *)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    knobs_data ret = {
        .rk = (rgb_knobs_value >> 16) & 0xFF,  // red knob position
        .gk = (rgb_knobs_value >> 8) & 0xFF,   // green knob position
        .bk = (rgb_knobs_value >> 0) & 0xFF,   // blue knob position
        .rb = (rgb_knobs_value >> 26) & 1,     // red button
        .gb = (rgb_knobs_value >> 25) & 1,     // green button
        .bb = (rgb_knobs_value >> 24) & 1      // blue button
    };
    return ret;
}
