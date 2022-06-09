#ifndef NOTES
#define NOTES

#include <math.h>

#define NUM_NOTES 107

typedef enum {
    Note_A0 = 0, Note_Bb0, Note_B0, 
    Note_C1, Note_Db1, Note_D1, Note_Eb1, Note_E1, Note_F1, Note_Gb1, Note_G1, Note_Ab1, Note_A1, Note_Bb1, Note_B1, 
    Note_C2, Note_Db2, Note_D2, Note_Eb2, Note_E2, Note_F2, Note_Gb2, Note_G2, Note_Ab2, Note_A2, Note_Bb2, Note_B2, 
    Note_C3, Note_Db3, Note_D3, Note_Eb3, Note_E3, Note_F3, Note_Gb3, Note_G3, Note_Ab3, Note_A3, Note_Bb3, Note_B3, 
    Note_C4, Note_Db4, Note_D4, Note_Eb4, Note_E4, Note_F4, Note_Gb4, Note_G4, Note_Ab4, Note_A4, Note_Bb4, Note_B4, 
    Note_C5, Note_Db5, Note_D5, Note_Eb5, Note_E5, Note_F5, Note_Gb5, Note_G5, Note_Ab5, Note_A5, Note_Bb5, Note_B5, 
    Note_C6, Note_Db6, Note_D6, Note_Eb6, Note_E6, Note_F6, Note_Gb6, Note_G6, Note_Ab6, Note_A6, Note_Bb6, Note_B6, 
    Note_C7, Note_Db7, Note_D7, Note_Eb7, Note_E7, Note_F7, Note_Gb7, Note_G7, Note_Ab7, Note_A7, Note_Bb7, Note_B7, 
    Note_C8, Note_Db8, Note_D8, Note_Eb8, Note_E8, Note_F8, Note_Gb8, Note_G8, Note_Ab8, Note_A8, Note_Bb8, Note_B8, 
    Note_C9, Note_Db9, Note_D9, Note_Eb9, Note_E9, Note_F9, Note_Gb9, Note_G9
} Note;

double frequency(Note note);

#endif