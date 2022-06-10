#include "stdin_monitor.h"

static int noteActive[NUM_NOTES]; 


void *monitorStdin(void *arg) {
    int err;
    char buf[4]; // Don't need bigger than this
    noncanonicalMode(STDIN_FILENO);
    setMin(STDIN_FILENO, 2);
    setTime(STDIN_FILENO, 0);
    for (int i = 0; i < NUM_NOTES; ++i) {
        noteActive[i] = 0;
    }
    while (1) {
        err = read(STDIN_FILENO, buf, 1); 
        Note note = charToNote(buf[0]);
        if ((0 <= note) && (note < NUM_NOTES)) {
            noteActive[(int)note] = !noteActive[(int)note];
        }
    }
    canonicalMode(STDIN_FILENO);
}

int isActiveStdin(Note note) {
    return noteActive[(int)note];
}

