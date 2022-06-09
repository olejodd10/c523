#ifndef NONCANON
#define NONCANON

#include <termios.h>
#include <unistd.h>

void canonicalMode(int fd);

void noncanonicalMode(int fd);

void setMin(int fd, cc_t min);

void setTime(int fd, cc_t time);

#endif