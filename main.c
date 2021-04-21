#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

bool lcd_initialization(void){
  bool ret = true;
  unsigned char *parlcd_mem_base;
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL)
    ret = false;

  parlcd_hx8357_init(parlcd_mem_base);

  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (int i = 0; i < 320 ; i++) {
    for (int j = 0; j < 480 ; j++) {
      parlcd_write_data(
        parlcd_mem_base, hsv2rgb_lcd(j, 255, (i*255)/320));
    }
  } return ret;
}

int main(int argc, char *argv[])
{
  if (lcd_initialization()){
    //continue
  }

  return EXIT_SUCCESS;
}
