#include <stdio.h>
#include "LCD_output.h"

int LCD_write_text(const char *text, int position_x, int position_y){
    // do something
    return 0;
}

int countdown(const char *text, int pos_x, int pos_y){
    for (int i = 0; i < 3; ++i){
        printf("%d", i);
    }
    return 0;
}
