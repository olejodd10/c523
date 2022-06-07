#include "notes.h"

const double A4_FREQUENCY = 440.0;

double frequency(Note note) {
    return A4_FREQUENCY*exp2((note-69.0)/12);
}