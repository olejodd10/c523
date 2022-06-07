#ifndef LPCM
#define LPCM

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double frequency;
    double amplitude;
    // double phase; // No point in having this
} SineWave;

double evaluate(SineWave wave, double fs, int k);

double evaluateCont(SineWave wave, double t);

double* sampleWave(SineWave wave, double fs, int n);

double* sampleWaves(SineWave* waves, int nWaves, double fs, int n);

double maxAmplitude(double* wave, int n);

void scale(double* wave, int n, double factor);

// https://en.wikipedia.org/wiki/Pulse-code_modulation
uint8_t* lpcm8bit(double* data, int length, double maxAmplitude);

#endif