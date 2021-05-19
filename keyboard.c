#include "keyboard.h"
#include <poll.h>

int keyboard_getc_timeout(int timeout_ms, unsigned char *c)
{
    int fd = 0; // stdin
    struct pollfd ufdr[1];
    int r = 0;
    ufdr[0].fd = fd; // stdin
    ufdr[0].events = POLLIN;
    if ((poll(&ufdr[0], 1, timeout_ms) > 0) && (ufdr[0].revents & (POLLIN))) {
        r = read(fd, c, 1);
    }
    return r;
}