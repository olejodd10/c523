#include "noncanon.h"

void canonicalMode(int fd) {
    struct termios t;
    tcgetattr(fd, &t);
    t.c_lflag |= ICANON;
    tcsetattr(fd, TCSANOW, &t);
}

void noncanonicalMode(int fd) {
    struct termios t;
    tcgetattr(fd, &t);
    t.c_lflag &= ~ICANON;
    tcsetattr(fd, TCSANOW, &t);
}

void setMin(int fd, cc_t min) {
    struct termios t;
    tcgetattr(fd, &t);
    t.c_cc[VMIN] = min;
    tcsetattr(fd, TCSANOW, &t);
}

void setTime(int fd, cc_t time) {
    struct termios t;
    tcgetattr(fd, &t);
    t.c_cc[VTIME] = time;
    tcsetattr(fd, TCSANOW, &t);
}