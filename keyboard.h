#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_TIMEOUT 100

/* 
 * wait KEYBOARD_TIMEOUT time for a character from stdin
 * returns 1 if a character has been read
 * returns 0 if no character has been read
 * returns -1 if an error occured
 */
int keyboard_getc_timeout(unsigned char *c);

#endif
