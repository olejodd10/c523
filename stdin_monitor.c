#include "stdin_monitor.h"

static int noteActive[NUM_NOTES]; 

// Inspired by virtualpiano.net's mapping
static Note charToNote(char c) {
    switch (c) {
    case '1':
        return Note_C1;
    case '!':
        return Note_Db1;
    case '2':
        return Note_D1;
    case '"':
        return Note_Eb1;
    case '3':
        return Note_E1;
    case '4':
        return Note_F1;
    case '$': // Sigh
        return Note_Gb1;
    case '5':
        return Note_G1;
    case '%':
        return Note_Ab1;
    case '6':
        return Note_A1;
    case '&':
        return Note_Bb1;
    case '7':
        return Note_B1;
    case '8':
        return Note_C2;
    case '(':
        return Note_Db2;
    case '9':
        return Note_D2;
    case ')':
        return Note_Eb2;
    case '0':
        return Note_E2;
    case 'q':
        return Note_F2;
    case 'Q':
        return Note_Gb2;
    case 'w':
        return Note_G2;
    case 'W':
        return Note_Ab2;
    case 'e':
        return Note_A2;
    case 'E':
        return Note_Bb2;
    case 'r':
        return Note_B2;
    case 't':
        return Note_C3;
    case 'T':
        return Note_Db3;
    case 'y':
        return Note_D3;
    case 'Y':
        return Note_Eb3;
    case 'u':
        return Note_E3;
    case 'i':
        return Note_F3;
    case 'I':
        return Note_Gb3;
    case 'o':
        return Note_G3;
    case 'O':
        return Note_Ab3;
    case 'p':
        return Note_A3;
    case 'P':
        return Note_Bb3;
    case 'a':
        return Note_B3;
    case 's':
        return Note_C4;
    case 'S':
        return Note_Db4;
    case 'd':
        return Note_D4;
    case 'D':
        return Note_Eb4;
    case 'f':
        return Note_E4;
    case 'g':
        return Note_F4;
    case 'G':
        return Note_Gb4;
    case 'h':
        return Note_G4;
    case 'H':
        return Note_Ab4;
    case 'j':
        return Note_A4;
    case 'J':
        return Note_Bb4;
    case 'k':
        return Note_B4;
    case 'l':
        return Note_C5;
    case 'L':
        return Note_Db5;
    case 'z':
        return Note_D5;
    case 'Z':
        return Note_Eb5;
    case 'x':
        return Note_E5;
    case 'c':
        return Note_F5;
    case 'C':
        return Note_Gb5;
    case 'v':
        return Note_G5;
    case 'V':
        return Note_Ab5;
    case 'b':
        return Note_A5;
    case 'B':
        return Note_Bb5;
    case 'n':
        return Note_B5;
    case 'm':
        return Note_C6;
    case 'M':
        return Note_Db6;
    default:
        return -1;
    }
}

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

int isActive(Note note) {
    return noteActive[(int)note];
}

