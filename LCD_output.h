#ifndef LCD_H
#define LCD_H

#include "headers.h"

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320
#define MENU_SELECTION 3
#define BACKGROUND_COLOR 0
#define MENU_OFFSET_X (DISPLAY_WIDTH / 3)
#define MENU_OFFSET_Y (DISPLAY_HEIGHT / 8)
#define CLEAN_CODE 0x2c
#define CLEAR_DISPLAY(lcd_mem_base) parlcd_write_cmd(lcd_mem_base, CLEAN_CODE);
#define SET_DISPLAY_BLACK                                                      \
    for (int h = 0; h < DISPLAY_HEIGHT; ++h) {                                 \
        for (int w = 0; w < DISPLAY_WIDTH; ++w) {                              \
            parlcd_write_data(                                                 \
                lcd_mem_base,                                                  \
                hsv2rgb_lcd(w, 255, (h * 255) / DISPLAY_HEIGHT));              \
        }                                                                      \
    }

bool display_init();
bool lcd_initialization(unsigned char **ret_lcd_mem_base);
unsigned int hsv2rgb_lcd(int hue, int saturation, int value);
int char_width(int ch);
void draw_char(int x, int y, char ch, unsigned short color, int scale, unsigned short *frame_buff);
void draw_pixel(int x, int y, unsigned short color, unsigned short *frame_buff);
void draw_pixel_big(int x, int y, int scale, unsigned short color, unsigned short *frame_buff);
void draw_string(int x, int y, unsigned short color, int scale, unsigned short *frame_buff, char *string);
void draw_grounded_string(int x, int y, int padding_x, int padding_y,
                          unsigned short color, unsigned short color_background,
                          int scale, unsigned short *frame_buff, char *string);
void update_display(unsigned short *frame_buff);
void print_menu(int x, int y, int selected, unsigned short *frame_buff);
void countdown(int x, int y, unsigned short *frame_buff);

#endif



