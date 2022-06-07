#include <stdint.h>
#include <stdio.h>
#include "notes.h"
#include "lpcm.h"

// https://www.alsa-project.org/alsa-doc/alsa-lib/pcm.html

int main() {
    double fs = 44100;
    SineWave waves[3] = {{frequency(Note_C5), 1.0}, {frequency(Note_E5), 1.0}, {frequency(Note_G5), 1.0}};
    double duration = 3.0;
    int n = (int)(duration*fs);
    double* data = sampleWaves(waves, 3, fs, n);
    uint8_t* byteWave = lpcm8bit(data, n, 3.0);
    FILE* file = fopen("mywave.pcm", "wb");
    fwrite(byteWave, sizeof(uint8_t), n, file);
}