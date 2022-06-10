#ifndef STDIN_MONITOR
#define STDIN_MONITOR

#include <stdio.h>
#include "noncanon.h"
#include "notes.h"

// Noncanonically uses stdin to keep track of whether notes are active

void *monitorStdin(void *arg);

int isActiveStdin(Note note);

#endif