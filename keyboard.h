#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_TIMEOUT 100

/* 
* wait KEYBOARD_TIMEOUT time for character from stdin
* KEYBOARD_TIMEOUT is defined in keyboard.c
* returns 1 if character has been read
* returns 0 if no character has been read
* returns -1 if error occured
 */
int keyboard_getc_timeout(unsigned char *c);

#endif
