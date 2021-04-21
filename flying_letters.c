/*******************************************************************
  Project main function template for MicroZed based MZ_APO board
  designed by Petr Porazil at PiKRON

  change_me.c      - main file

  include your name there and license for distribution.

  Remove next text: This line should not appear in submitted
  work and project name should be change to match real application.
  If this text is there I want 10 points subtracted from final
  evaluation.

 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"

#define M_PI 3.1415

unsigned int hsv2rgb_lcd(int hue, int saturation, int value) {
  hue = (hue%360);    
  float f = ((hue%60)/60.0);
  int p = (value*(255-saturation))/255;
  int q = (value*(255-(saturation*f)))/255;
  int t = (value*(255-(saturation*(1.0-f))))/255;
  unsigned int r,g,b;
  if (hue < 60){
    r = value; g = t; b = p;
  } else if (hue < 120) {
    r = q; g = value; b = p;
  } else if (hue < 180) {
    r = p; g = value; b = t;
  } else if (hue < 240) {
    r = p; g = q; b = value;
  } else if (hue < 300) {
    r = t; g = p; b = value;
  } else {
    r = value; g = p; b = q;
  }
  r>>=3;
  g>>=2;
  b>>=3;
  return (((r&0x1f)<<11)|((g&0x3f)<<5)|(b&0x1f));
}

unsigned short *fb;
font_descriptor_t *fdes;
int scale=4;

void draw_pixel(int x, int y, unsigned short color) {
  if (x>=0 && x<480 && y>=0 && y<320) {
    fb[x+480*y] = color;
  }
}

void draw_pixel_big(int x, int y, unsigned short color) {
  int i,j;
  for (i=0; i<scale; i++) {
    for (j=0; j<scale; j++) {
      draw_pixel(x+i, y+j, color);
    }
  }
}

int char_width(int ch) {
  int width;
  if (!fdes->width) {
    width = fdes->maxwidth;
  } else {
    width = fdes->width[ch-fdes->firstchar];
  }
  return width;
}

void draw_char(int x, int y, char ch, unsigned short color) {
  int w = char_width(ch);
  const font_bits_t *ptr;
  if ((ch >= fdes->firstchar) && (ch-fdes->firstchar < fdes->size)) {
    if (fdes->offset) {
      ptr = &fdes->bits[fdes->offset[ch-fdes->firstchar]];
    } else {
      int bw = (fdes->maxwidth+15)/16;
      ptr = &fdes->bits[(ch-fdes->firstchar)*bw*fdes->height];
    }
    int i, j;
    for (i=0; i<fdes->height; i++) {
      font_bits_t val = *ptr;
      for (j=0; j<w; j++) {
        if ((val&0x8000)!=0) {
          draw_pixel_big(x+scale*j, y+scale*i, color);
        }
        val<<=1;
      }
      ptr++;
    }
  }
}


int main(int argc, char *argv[]) {
  unsigned char *parlcd_mem_base;
  int i,j;
  printf("Hello\n");
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL)
    exit(1);

  parlcd_hx8357_init(parlcd_mem_base);

  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (i = 0; i < 320 ; i++) {
    for (j = 0; j < 480 ; j++) {
      parlcd_write_data(
        parlcd_mem_base, hsv2rgb_lcd(j, 255, (i*255)/320));
    }
  }
  sleep(5);
  
  int k;
  int ptr;
  struct timespec loop_delay = 
    {.tv_sec = 0, .tv_nsec = 120 * 1000 * 1000};
  

  fdes = &font_winFreeSystem14x16;
  fb  = (unsigned short *)malloc(320*480*2);
 
  float g=1.0;
  for (k=0; k<=80; k+=5) {
    float alfa=((10+k)*M_PI)/180.0;
    float vx=32*(M_PI/2.0-alfa);
    float vy=32*(2.0*alfa/M_PI);
    float x=1;
    float y=1;
    char ch=('a'+k/5);
    unsigned int col=hsv2rgb_lcd(4*k,255,255);
    while ((x<480) && (y>0)) {
      for (ptr = 0; ptr < 320*480 ; ptr++) {
        fb[ptr]=0;
      }
      draw_char((int)x,250-(int)y, ch, col);
      x+=vx;
      y+=vy;
      vx = vx*0.97;
      vy = vy*0.97-g;
      parlcd_write_cmd(parlcd_mem_base, 0x2c);
      for (ptr = 0; ptr < 480*320 ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
      }
      clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
  }
  ptr=0;
  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (i = 0; i < 320 ; i++) {
    for (j = 0; j < 480 ; j++) {
      fb[ptr]=0;
      parlcd_write_data(parlcd_mem_base, fb[ptr++]);
    }
  }

  printf("Goodbye\n");

  return 0;
}
