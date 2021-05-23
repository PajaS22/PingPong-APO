#ifndef LCD_H
#define LCD_H

#include "headers.h"

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320
#define MENU_SELECTION 3
#define PAUSE_MENU_SELECTION 1
#define BACKGROUND_COLOR 0
#define MENU_OFFSET_X (DISPLAY_WIDTH / 3)
#define MENU_OFFSET_Y (DISPLAY_HEIGHT / 8)
#define PAUSE_MENU_OFFSET_X (DISPLAY_WIDTH / 3)
#define PAUSE_MENU_OFFSET_Y (DISPLAY_HEIGHT / 8)
#define CLEAN_CODE 0x2c
#define CLEAR_DISPLAY(lcd_mem_base) parlcd_write_cmd(lcd_mem_base, CLEAN_CODE);
#define BLACK 0
#define WHITE hsv2rgb_lcd(0, 0, 255)
#define RED hsv2rgb_lcd(5, 235, 255)
#define GREEN hsv2rgb_lcd(145, 196, 255)
#define BLUE hsv2rgb_lcd(199, 230, 255)
#define PURPLE hsv2rgb_lcd(282, 100, 255)

bool display_init();
bool lcd_initialization(unsigned char **ret_lcd_mem_base);
void set_display_black(ushort *frame_buff);
void clear_buffer(ushort *frame_buff);
void update_display(ushort *frame_buff);
unsigned int hsv2rgb_lcd(int hue, int saturation, int value);
int char_width(int ch);
void draw_char(int x, int y, char ch, ushort color, int scale, ushort *frame_buff);
void draw_pixel(int x, int y, ushort color, ushort *frame_buff);
void draw_pixel_big(int x, int y, int scale, ushort color, ushort *frame_buff);
void draw_string(int x, int y, ushort color, int scale, ushort *frame_buff, char *string);
void draw_grounded_string(int x, int y, int padding_x, int padding_y, ushort color, 
                         ushort color_back, int scale, ushort *frame_buff, char *string);
void print_menu(int x, int y, int selected, ushort *frame_buff);
void print_pause_menu(int x, int y, int selected, ushort *frame_buff);
void countdown(int x, int y, int scale, ushort *frame_buff);
void goodbye(ushort *frame_buff);
void print_congrats(ushort *frame_buff, ushort color);

#endif
