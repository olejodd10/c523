#include "notes.h"

static const double A4_FREQUENCY = 440.0;

double frequency(Note note) {
    return A4_FREQUENCY*exp2(((double)note-Note_A4)/12.); // Example of why casting is important. Inf otherwise.
}