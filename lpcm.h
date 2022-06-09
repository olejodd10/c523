#ifndef LPCM
#define LPCM

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double frequency;
    double amplitude;
    double phase; 
} SineWave;

double evaluate(SineWave wave, double fs, int k);

void shiftPhase(SineWave* wave, double fs, int k);

void layer(SineWave wave, double* data, double fs, int n);

void layerLinearFadeIn(SineWave wave, double* data, double fs, int n, int fadeLen);

void layerLinearFadeOut(SineWave wave, double* data, double fs, int n, int fadeLen);

double maxAmplitude(double* wave, int n);

void scale(double* wave, int n, double factor);

// https://en.wikipedia.org/wiki/Pulse-code_modulation
void lpcmS16(double* inData, int16_t* outData, int length, double maxAmplitude);

#endif