#define _POSIX_C_SOURCE 200112L

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "font_types.h"
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#define CLEAN_CODE 0x2c
#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGTH 320
#define CLEAR_DISPLAY(lcd_mem_base) parlcd_write_cmd(lcd_mem_base, CLEAN_CODE);
#define SET_DISPLAY_BLACK                                                      \
    for (int h = 0; h < DISPLAY_HEIGTH; ++h) {                                 \
        for (int w = 0; w < DISPLAY_WIDTH; ++w) {                              \
            parlcd_write_data(                                                 \
                lcd_mem_base,                                                  \
                hsv2rgb_lcd(w, 255, (h * 255) / DISPLAY_HEIGTH));              \
        }                                                                      \
    }

unsigned int hsv2rgb_lcd(int hue, int saturation, int value);
bool lcd_initialization(unsigned char **ret_lcd_mem_base);
int char_width(int ch);
void draw_pixel_big(int x, int y, int scale, unsigned short color, unsigned short *frame_buff);
void draw_pixel(int x, int y, unsigned short color, unsigned short *frame_buff);
void draw_char(int x, int y, char ch, unsigned short color, int scale, unsigned short *frame_buff);
void draw_string(int x, int y, unsigned short color, int scale, unsigned short *frame_buff, char *string);

void draw_grounded_string(int x, int y, int padding_x, int padding_y, unsigned short color,
    unsigned short color_background, int scale, unsigned short *frame_buff, char *string);
void print_menu(int x, int y, int selected, unsigned short *frame_buff);

font_descriptor_t *fdes;

int main(int argc, char *argv[]){
    unsigned char *lcd_mem_base;
    if (lcd_initialization(&lcd_mem_base)) {
        unsigned short *frame_buff;
        frame_buff = (unsigned short *)malloc(DISPLAY_HEIGTH * DISPLAY_WIDTH *
                                      sizeof(unsigned short));
        fdes = &font_winFreeSystem14x16;
        if (frame_buff == NULL) {
            fprintf(stderr, "ERROR: Memory could not be allocated!\n");
        } else {
            /*
            draw_char(110, 25, 'M', hsv2rgb_lcd(0, 0, 0), 5, frame_buff);
            draw_char(190, 25, 'E', hsv2rgb_lcd(0, 0, 0), 5, frame_buff);
            draw_char(250, 25, 'N', hsv2rgb_lcd(0, 0, 0), 5, frame_buff);
            draw_char(320, 25, 'U', hsv2rgb_lcd(0, 0, 0), 5, frame_buff);
            */
            print_menu(DISPLAY_WIDTH/3, DISPLAY_HEIGTH/8, 3, frame_buff);
            //draw_string(10, 10, 0, 5, frame_buff, "Hello world!");
            //draw_grounded_string(70, 200, 2, 2, 0, hsv2rgb_lcd(120, 255, 255), 3, frame_buff, "Hello world!");

            parlcd_write_cmd(lcd_mem_base, CLEAN_CODE);
            for (int ptr = 0; ptr < DISPLAY_WIDTH * DISPLAY_HEIGTH; ++ptr) {
                parlcd_write_data(lcd_mem_base, frame_buff[ptr]);
            }
        }
    }

    return EXIT_SUCCESS;
}

bool lcd_initialization(unsigned char **ret_lcd_mem_base){
    bool ret = true;
    unsigned char *lcd_mem_base;
    lcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    if (lcd_mem_base == NULL)
        ret = false;

    parlcd_hx8357_init(lcd_mem_base);

    CLEAR_DISPLAY(lcd_mem_base);
    SET_DISPLAY_BLACK
    *ret_lcd_mem_base = lcd_mem_base;
    return ret;
}

unsigned int hsv2rgb_lcd(int hue, int saturation, int value){
    hue = (hue % 360);
    float f = ((hue % 60) / 60.0);
    int p = (value * (255 - saturation)) / 255;
    int q = (value * (255 - (saturation * f))) / 255;
    int t = (value * (255 - (saturation * (1.0 - f)))) / 255;
    unsigned int r, g, b;
    if (hue < 60) {
        r = value;
        g = t;
        b = p;
    } else if (hue < 120) {
        r = q;
        g = value;
        b = p;
    } else if (hue < 180) {
        r = p;
        g = value;
        b = t;
    } else if (hue < 240) {
        r = p;
        g = q;
        b = value;
    } else if (hue < 300) {
        r = t;
        g = p;
        b = value;
    } else {
        r = value;
        g = p;
        b = q;
    }
    r >>= 3;
    g >>= 2;
    b >>= 3;
    return (((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f));
}

void draw_char(int x, int y, char ch, unsigned short color, int scale, unsigned short *frame_buff){
    int w = char_width(ch);
    const font_bits_t *ptr_data;
    if ((ch >= fdes->firstchar) && (ch - fdes->firstchar < fdes->size)) { //the char is within the struct
        if (fdes->offset) { //letters' offsets are defined
            ptr_data = &fdes->bits[fdes->offset[ch - fdes->firstchar]];
        } else {
            int bw = (fdes->maxwidth + 15) / 16; // ???
            ptr_data = &fdes->bits[(ch - fdes->firstchar) * bw * fdes->height];
        }
        for (int i = 0; i < fdes->height; i++) {
            font_bits_t val = *ptr_data;
            for (int j = 0; j < w; j++) { //every column
                if ((val & 0x8000) != 0) { //0000 1000 0000 0000 0000
                    draw_pixel_big(x + scale * j, y + scale * i, scale, color, frame_buff);
                } val <<= 1;
            } ptr_data++;
        }
    }
}

void draw_pixel(int x, int y, unsigned short color, unsigned short *frame_buff){
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGTH) {
        frame_buff[x + DISPLAY_WIDTH * y] = color;
    }
}

void draw_pixel_big(int x, int y, int scale, unsigned short color, unsigned short *frame_buff){
    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            draw_pixel(x + i, y + j, color, frame_buff);
        }
    }
}

int char_width(int ch){
    int width;
    if (!fdes->width) {
        width = fdes->maxwidth;
    } else {
        width = fdes->width[ch - fdes->firstchar];
    }
    return width;
}

void draw_string(int x, int y, unsigned short color, int scale, unsigned short *frame_buff, char *string){
    int w;
    for(char *c = string; *c != '\0'; c++){
        w = char_width(*c)*scale;
        if(x + w >=  DISPLAY_WIDTH){
            // out of display
            break;
        }
        draw_char(x, y, *c, color, scale, frame_buff);
        x += w;
    }
}

void draw_grounded_string(int x, int y, int padding_x, int padding_y, unsigned short color,
    unsigned short color_background, int scale, unsigned short *frame_buff, char *string) {
    int width = 0;
    for(char *c = string; *c != '\0'; c++){
        width += char_width(*c);
    }
    width += padding_x * 2;
    int height = fdes->height + 2 * padding_y;
    int ground_x = x - padding_x;
    int ground_y = y - padding_y;
    if (ground_x >= 0 && ground_y >= 0 && ground_x + width < DISPLAY_WIDTH && ground_y + height < DISPLAY_HEIGTH){
        // print ground
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                draw_pixel_big(ground_x + w*scale, ground_y + h*scale, scale, color_background, frame_buff);
            }
        }
    }
    draw_string(x, y, color, scale, frame_buff, string);
}

void print_menu(int x, int y, int selected, unsigned short *frame_buff) {
    char *menu[] = {"MENU", "Normal", "Hard", "Exit"};
    int menu_num = sizeof(menu)/sizeof(char*);
    int line_padding = 3;
    int ground_padding = 2;
    unsigned short color = hsv2rgb_lcd(0, 0, 255);
    unsigned short color_on_ground = hsv2rgb_lcd(0, 0, 0); 
    unsigned short color_background = hsv2rgb_lcd(120, 255, 255);
    int scale = 3;
    for (int i = 0; i < menu_num; ++i){
        printf("item %d\n", i);
        if (i == selected)
            draw_grounded_string(x, y, ground_padding, ground_padding, color_on_ground, color_background, scale, frame_buff, menu[i]);
        else 
            draw_string(x, y, color, scale, frame_buff, menu[i]);
        y += (fdes->height + line_padding) * scale;
    }
}