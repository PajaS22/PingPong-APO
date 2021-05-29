#ifndef LCD_H
#define LCD_H

#include "headers.h"

// COLORS
#define BLACK 0
#define WHITE hsv2rgb_lcd(0, 0, 255)
#define RED hsv2rgb_lcd(5, 235, 255)
#define GREEN hsv2rgb_lcd(145, 196, 255)
#define BLUE hsv2rgb_lcd(199, 230, 255)
#define PURPLE hsv2rgb_lcd(282, 100, 255)

// DISPLAY CONSTANTS
#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320
#define BACKGROUND_COLOR BLACK

// MAIN MENU
#define MENU_SELECTION 3 // number of options in the main menu
#define MENU_OFFSET_X (DISPLAY_WIDTH / 3)
#define MENU_OFFSET_Y (DISPLAY_HEIGHT / 8)

// PAUSE MENU
#define PAUSE_MENU_SELECTION 1 // maximal index of options in the pause menu
#define PAUSE_MENU_OFFSET_X (DISPLAY_WIDTH / 3)
#define PAUSE_MENU_OFFSET_Y (DISPLAY_HEIGHT / 8)

// DISPLAY CLEANING
#define CLEAN_CODE 0x2c
// writes a clearing command to the right memory adress
#define CLEAR_DISPLAY(lcd_mem_base) parlcd_write_cmd(lcd_mem_base, CLEAN_CODE);

/* calls lcd_initialization() and sets fonts for the text printing */
bool display_init();

/* sets the display base memory adress and clears the display */
bool lcd_initialization(unsigned char **ret_lcd_mem_base);

/* sets all pixels black */
void set_display_black(pixel *frame_buff);

/* sets all pixels in the buffer to zero */
void clear_buffer(pixel *frame_buff);

/* prints the frame buffer to the LCD */
void update_display(pixel *frame_buff);

/* returns an RGB value of the color given in hue syntax */
unsigned int hsv2rgb_lcd(int hue, int saturation, int value);

/* returns the width of character in a set font in pixels */
int char_width(int ch);

/* prints a character to the frame buffer */
void draw_char(int x, int y, char ch, pixel color, int scale, pixel *frame_buff);

/* prints a pixel to the frame buffer */
void draw_pixel(int x, int y, pixel color, pixel *frame_buff);

/* 
 * prints a pixel multiplied by a scale to the frame buffer
 * example: one pixel multiplied by scale = 4 would be represented as a square of four pixels
 */
void draw_pixel_big(int x, int y, int scale, pixel color, pixel *frame_buff);

/* prints a string at the end to the frame buffer */
void draw_string(int x, int y, pixel color, int scale, pixel *frame_buff, char *string);

/* prints a string with a rectangle of the given color in the background */
void draw_grounded_string(int x, int y, int padding_x, int padding_y, pixel color, pixel color_back, int scale, pixel *frame_buff, char *string);

/* 
 * prints the main menu
 * higlights the 'selected' option
 */
void print_menu(int x, int y, int selected, pixel *frame_buff);

/* 
 * prints the pause menu 
 * higlights the 'selected' option
 */
void print_pause_menu(int x, int y, int selected, pixel *frame_buff);

/* prints 3, 2, 1 with the COUNTDOWN_SLEEP period */
void countdown(int x, int y, int scale, pixel *frame_buff);

/* prints goodbye */
void goodbye(pixel *frame_buff);

/* prints a congratulation or a draw */
void print_congrats(pixel *frame_buff, pixel color);

#endif
