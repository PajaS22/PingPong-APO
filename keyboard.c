#include "keyboard.h"
#include <poll.h>
#include "headers.h"

#define KEYBOARD_TIMEOUT 100

int keyboard_getc_timeout(unsigned char *c) {
    int fd = 0;  // stdin
    struct pollfd ufdr[1];
    int r = 0;
    ufdr[0].fd = fd;  // stdin
    ufdr[0].events = POLLIN;
    if ((poll(&ufdr[0], 1, KEYBOARD_TIMEOUT) > 0) && (ufdr[0].revents & (POLLIN))) 
        r = read(fd, c, 1);
    return r;
}