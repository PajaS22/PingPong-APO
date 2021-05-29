#include "LCD_output.h"
#include "bonus.h"
#include "font_types.h"
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#define COUNTDOWN_SLEEP 1000000 // 1 second
#define GOODBYE_SLEEP 2000000   // 2 seconds
#define CONGRATS_SLEEP 2000000  // 2 seconds 

static unsigned char *lcd_mem_base;
static font_descriptor_t *fdes;

static char *player_red_wins = " RED WINS!";
static char *player_blue_wins = "BLUE WINS!";
static char *draw = "IT'S A DRAW!";
static char *menu[] = { "PING PONG", "Normal", "Hard", "Exit" };
static char *pause_menu[] = {"Pause menu", "Resume", "Exit"};
static int countdown_ints[] = { 3, 2, 1 };
static char *start = "START!";
static char *greeting = "GOODBYE!";

bool display_init() {
    bool ret = lcd_initialization(&lcd_mem_base);
    fdes = &font_winFreeSystem14x16;
    return ret;
}

bool lcd_initialization(unsigned char **ret_lcd_mem_base) {
    bool ret = true;
    lcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    if (lcd_mem_base == NULL) ret = false;

    parlcd_hx8357_init(lcd_mem_base);

    CLEAR_DISPLAY(lcd_mem_base);
    *ret_lcd_mem_base = lcd_mem_base;
    return ret;
}

void set_display_black(pixel *frame_buff) {
    for (int h = 0; h < DISPLAY_HEIGHT; ++h) {
        for (int w = 0; w < DISPLAY_WIDTH; ++w) {
            parlcd_write_data(lcd_mem_base, BLACK);
            frame_buff[w + h * DISPLAY_WIDTH] = BLACK;
        }
    }
}

void clear_buffer(pixel *frame_buff) {
    for (int h = 0; h < DISPLAY_HEIGHT; ++h) {
        for (int w = 0; w < DISPLAY_WIDTH; ++w) {
            frame_buff[w + h * DISPLAY_WIDTH] = BLACK;
        }
    }
}

void update_display(pixel *frame_buff) {
    parlcd_write_cmd(lcd_mem_base, CLEAN_CODE);
    for (int ptr = 0; ptr < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++ptr) {
        parlcd_write_data(lcd_mem_base, frame_buff[ptr]);
    }
}

unsigned int hsv2rgb_lcd(int hue, int saturation, int value) {
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

int char_width(int ch) {
    int width;
    if (!fdes->width) {
        width = fdes->maxwidth;
    } else {
        width = fdes->width[ch - fdes->firstchar];
    }
    return width;
}

void draw_char(int x, int y, char ch, pixel color, int scale, pixel *frame_buff) {
    int w = char_width(ch);
    const font_bits_t *ptr_data;
    if ((ch >= fdes->firstchar) && (ch - fdes->firstchar < fdes->size)) {  // the char is within the struct
        if (fdes->offset) {                                                // letters' offsets are defined
            ptr_data = &fdes->bits[fdes->offset[ch - fdes->firstchar]];
        } else {
            int bw = (fdes->maxwidth + 15) / 16;
            ptr_data = &fdes->bits[(ch - fdes->firstchar) * bw * fdes->height];
        }
        for (int i = 0; i < fdes->height; i++) {
            font_bits_t val = *ptr_data;
            for (int j = 0; j < w; j++) {
                if ((val & 0x8000) != 0) {  // 1000 0000 0000 0000
                    draw_pixel_big(x + scale * j, y + scale * i, scale, color, frame_buff);
                }
                val <<= 1;
            }
            ptr_data++;
        }
    }
}

void draw_pixel(int x, int y, pixel color, pixel *frame_buff) {
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT) { frame_buff[x + DISPLAY_WIDTH * y] = color; }
}

void draw_pixel_big(int x, int y, int scale, pixel color, pixel *frame_buff) {
    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            draw_pixel(x + i, y + j, color, frame_buff);
        }
    }
}

void draw_string(int x, int y, pixel color, int scale, pixel *frame_buff, char *string) {
    int w;
    for (char *c = string; *c != '\0'; c++) {
        w = char_width(*c) * scale;
        if (x + w >= DISPLAY_WIDTH) {
            // out of display
            break;
        }
        draw_char(x, y, *c, color, scale, frame_buff);
        x += w;
    }
}

void draw_grounded_string(int x, int y, int padding_x, int padding_y, pixel color, pixel color_background, int scale, pixel *frame_buff, char *string) {
    int width = 0;
    for (char *c = string; *c != '\0'; c++) {
        width += char_width(*c);
    }
    width += padding_x * 2;
    int height = fdes->height + 2 * padding_y;
    int ground_x = x - padding_x;
    int ground_y = y - padding_y;
    if (ground_x >= 0 && ground_y >= 0 && ground_x + width < DISPLAY_WIDTH && ground_y + height < DISPLAY_HEIGHT) {
        // print ground
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                draw_pixel_big(ground_x + w * scale, ground_y + h * scale, scale, color_background, frame_buff);
            }
        }
    }
    draw_string(x, y, color, scale, frame_buff, string);
}

void print_menu(int x, int y, int selected, pixel *frame_buff) {
    int menu_num = sizeof(menu) / sizeof(char*);
    int line_padding = 3;
    int ground_padding = 1;
    pixel color = WHITE;
    pixel color_on_ground = BLACK;
    pixel color_background = GREEN;
    int center_x = -50;

    int scale = 3;
    for (int i = 0; i < menu_num; ++i) {
        if (i > 0) center_x = 0;
        if (i == selected + 1)
            draw_grounded_string(x + center_x, y, ground_padding, ground_padding, color_on_ground, color_background, scale, frame_buff, menu[i]);
        else
            draw_grounded_string(x + center_x, y, ground_padding, ground_padding, color, BACKGROUND_COLOR, scale, frame_buff, menu[i]);
        y += (fdes->height + line_padding) * scale;
    }
}

void print_pause_menu(int x, int y, int selected, pixel *frame_buff) {
    int menu_num = sizeof(pause_menu) / sizeof(char *);
    int line_padding = 3;
    int ground_padding = 1;
    pixel color = WHITE;
    pixel color_on_ground = BLACK;
    pixel color_background = GREEN;
    int center_x = -30;

    int scale = 3;
    for (int i = 0; i < menu_num; ++i) {
        if (i > 0) center_x = 0;
        if (i == selected + 1)
            draw_grounded_string(x + center_x, y, ground_padding, ground_padding, color_on_ground, color_background, scale, frame_buff, pause_menu[i]);
        else
            draw_grounded_string(x + center_x, y, ground_padding, ground_padding, color, BACKGROUND_COLOR, scale, frame_buff, pause_menu[i]);
        y += (fdes->height + line_padding) * scale;
    }
}

void countdown(int x, int y, int scale, pixel *frame_buff) {
    int ground_padding = 2;
    char tmp[2];
    int countdown_num = sizeof(countdown_ints) / sizeof(int);
    pixel color = WHITE;
    for (int i = 0; i < countdown_num; ++i) {
        sprintf(tmp, "%d", countdown_ints[i]);
        draw_grounded_string(x + 110, y, ground_padding, ground_padding, color, BACKGROUND_COLOR, scale, frame_buff, tmp);
        update_display(frame_buff);
        usleep(COUNTDOWN_SLEEP);
    }
    draw_grounded_string(x, y, ground_padding, ground_padding, color, BACKGROUND_COLOR, scale, frame_buff, start);
    update_display(frame_buff);
    usleep(COUNTDOWN_SLEEP);
    draw_grounded_string(x, y, ground_padding, ground_padding, BACKGROUND_COLOR, BACKGROUND_COLOR, scale, frame_buff, start);
    update_display(frame_buff);
}

void goodbye(pixel *frame_buff) {
    int ground_padding = 2;
    pixel color = WHITE;
    int scale = 4;
    int x = 90;
    int y = 120;
    set_display_black(frame_buff);
    draw_grounded_string(x, y, ground_padding, ground_padding, color, BACKGROUND_COLOR, scale, frame_buff, greeting);
    update_display(frame_buff);
    usleep(GOODBYE_SLEEP);
    set_display_black(frame_buff);
    update_display(frame_buff);
}

void print_congrats(pixel *frame_buff, pixel color) {
    int ground_padding = 2;
    int scale = 4;
    int x = 80;
    int y = 120;
    set_display_black(frame_buff);
    char *winner;
    if (color == RED)
        winner = player_red_wins;
    else if (color == BLUE)
        winner = player_blue_wins;
    else
        winner = draw;

    draw_grounded_string(x, y, ground_padding, ground_padding, color, BACKGROUND_COLOR, scale, frame_buff, winner);
    update_display(frame_buff);
    usleep(CONGRATS_SLEEP);
    set_display_black(frame_buff);
    update_display(frame_buff);
}
