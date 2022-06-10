#ifndef TCP_MONITOR
#define TCP_MONITOR

#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "notes.h"

void *monitorTcp(void *arg);

int isActiveTcp(Note note);

#endif