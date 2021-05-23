#include "LED.h"
#include "game.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#define RED_LED 0x00FF0000
#define BLUE_LED 0x000000FF
#define SWITCH_OFF 0x0

static unsigned char *led_mem_base;

bool led_init() {
    bool ret = true;
    led_mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (!led_mem_base) {
        fprintf(stderr, "ERROR: Physical address could not be allocated!\n");
        ret = false;
    } else {
        led_line(NO_GOAL, NO_GOAL);
        goal_lights(NO_GOAL);
    }
    return ret;
}

void goal_lights(int goal) {
    if (goal == LEFT_GOAL)
        *(volatile uint32_t *)(led_mem_base + SPILED_REG_LED_RGB1_o) = RED_LED;
    else if (goal == RIGHT_GOAL)
        *(volatile uint32_t *)(led_mem_base + SPILED_REG_LED_RGB2_o) = BLUE_LED;
    else {
        *(volatile uint32_t *)(led_mem_base + SPILED_REG_LED_RGB1_o) = SWITCH_OFF;
        *(volatile uint32_t *)(led_mem_base + SPILED_REG_LED_RGB2_o) = SWITCH_OFF;
    }
}

void led_line(int goals_l, int goals_r) {
    if (goals_l <= ALL_GOALS && goals_r <= ALL_GOALS) {
        int l = 0;
        short r = 0;

        unsigned int tmp = 1;
        for (int i = 0; i < goals_r; ++i) {
            r |= tmp;
            tmp = tmp << 1;
        }
        tmp = 1 << (NUM_LEDS_LINE - 1);
        for (int i = 0; i < goals_l; ++i) {
            l |= tmp;
            tmp = tmp >> 1;
        }
        *(volatile uint32_t *)(led_mem_base + SPILED_REG_LED_LINE_o) = l | r;
    }
}

void led_line_reset() {
    *(volatile uint32_t *)(led_mem_base + SPILED_REG_LED_LINE_o) = 0;
}

void blick_n_times(int n, int period, int colour) {
    for (int i = 0; i < n; ++i) {
        goal_lights(colour);
        usleep(period);
        goal_lights(NO_GOAL);
        usleep(period);
    }
}
