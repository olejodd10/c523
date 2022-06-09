#include "lpcm.h"

double evaluate(SineWave wave, double fs, int k) {
    return wave.amplitude*sin(2*M_PI*k*wave.frequency/fs + wave.phase);
}

void shiftPhase(SineWave* wave, double fs, int k) {
    wave->phase = fmod(wave->phase + 2*M_PI*k*wave->frequency/fs, 2*M_PI);
}


void layer(SineWave wave, double* data, double fs, int n) {
    for (int k = 0; k < n; ++k) {
        data[k] += evaluate(wave, fs, k);
    }
}

void layerLinearFadeIn(SineWave wave, double* data, double fs, int n, int fadeLen) {
    for (int k = 0; k < n; ++k) {
        data[k] += fmin(1.0, (double)k/fadeLen)*evaluate(wave, fs, k);
    }
}

void layerLinearFadeOut(SineWave wave, double* data, double fs, int n, int fadeLen) {
    for (int k = 0; k < (n < fadeLen ? n : fadeLen); ++k) {
        data[k] += ((double)(fadeLen-k)/fadeLen)*evaluate(wave, fs, k);
    }
}

double maxAmplitude(double* wave, int n) {
    double max = 0.0;
    for (int i = 0; i < n; ++i) {
        max = fmax(fabs(wave[i]), max);
    }
    return max;
}

void scale(double* wave, int n, double factor) {
    for (int i = 0; i < n; ++i) {
        wave[i] *= factor;
    }
}

// Clips samples at [-maxAmplitude, maxAmplitude]
void lpcmS16(double* inData, int16_t* outData, int length, double maxAmplitude) {
    for (int i = 0; i < length; ++i) {
        double clippedSample = fmax(fmin(maxAmplitude, inData[i]), -maxAmplitude); // TEST THIS
        outData[i] = (clippedSample/maxAmplitude) * INT16_MAX;
    }
}
