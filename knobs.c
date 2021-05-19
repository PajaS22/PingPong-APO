// handle knobs input
#include "knobs.h"
#include "mzapo_regs.h"

static unsigned char *mem_base;

bool knobs_init() {
    bool ret = true;
    mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (mem_base == NULL) {
        fprintf(stderr, "ERROR: Physical address could not be allocated!\n");
        ret = false;
    }
    return ret;
}

// returns actual knobs position
knobs_data knobs_value() {
    uint32_t rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

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
