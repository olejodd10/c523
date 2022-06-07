#include "lpcm.h"

double evaluate(SineWave wave, double fs, int k) {
    return wave.amplitude*sin(2*M_PI*k*wave.frequency/fs);
}

double evaluateCont(SineWave wave, double t) {
    return wave.amplitude*sin(2*M_PI*wave.frequency*t);
}

double* sampleWave(SineWave wave, double fs, int n) {
    double* data = malloc(sizeof(double)*n);
    for (int k = 0; k < n; ++k) {
        data[k] = evaluate(wave, fs, k);
    }
    return data;
}

double* sampleWaves(SineWave* waves, int nWaves, double fs, int n) {
    double* data = malloc(sizeof(double)*n);
    for (int k = 0; k < n; ++k) {
        data[k] = 0;
        for (int i = 0; i < nWaves; ++i) {
            data[k] += evaluate(waves[i], fs, k);
        }
    }
    return data;
}

double maxAmplitude(double* wave, int n) {
    double max = 0.0;
    for (int i = 0; i < n; ++i) {
        max = fmax(abs(wave[i]), max);
    }
    return max;
}

void scale(double* wave, int n, double factor) {
    for (int i = 0; i < n; ++i) {
        wave[i] *= factor;
    }
}

// https://en.wikipedia.org/wiki/Pulse-code_modulation
// Clips samples at [-maxAmplitude, maxAmplitude]
uint8_t* lpcm8bit(double* data, int length, double maxAmplitude) {
    uint8_t* pcmData = malloc(sizeof(uint8_t)*length);
    for (int i = 0; i < length; ++i) {
        double clippedSample = fmax(fmin(maxAmplitude, data[i]), -maxAmplitude); // TEST THIS
        pcmData[i] = ((clippedSample/maxAmplitude + 1)/2) * 255; // Linear mapping to 0..255
    }
    return pcmData;
}