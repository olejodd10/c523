#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include "alsa_player.h"
#include "stdin_monitor.h"
#include "tcp_monitor.h"
 
int main() {

    snd_pcm_t *handle;
    int err;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
 
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);
 
    snd_output_t *output = NULL;

    err = snd_output_stdio_attach(&output, stdout, 0);
    if (err < 0) {
        printf("Output failed: %s\n", snd_strerror(err));
        return 0;
    }

    printf("Playback device is %s\n", DEVICE);
    printf("Stream parameters are %uHz, %s, %u channels\n", STREAM_RATE, snd_pcm_format_name(FORMAT), CHANNELS);
 
    if ((err = snd_pcm_open(&handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) { 
        printf("Playback open error: %s\n", snd_strerror(err));
        return 0;
    }
    
    if ((err = setHWParams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) { 
        printf("Setting of hwparams failed: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = setSWParams(handle, swparams)) < 0) {
        printf("Setting of swparams failed: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
 
    if (VERBOSE)
        snd_pcm_dump(handle, output);

    double* floatData = calloc(PERIOD_LEN, sizeof(double));
    if (floatData == NULL) {
        printf("No enough memory\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < PERIOD_LEN; ++i) {
        floatData[i] = 0.;
    }
    
    int16_t* byteData = calloc(PERIOD_LEN, sizeof(int16_t));
    if (byteData == NULL) {
        printf("No enough memory\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < PERIOD_LEN; ++i) {
        byteData[i] = 0; // ikke egentlig nødvendig
    }
 
    pthread_t thread;
    pthread_create(&thread, NULL, monitorStdin, NULL);

    err = streamNotes(handle, floatData, byteData, isActiveStdin);
    if (err < 0)
        printf("Transfer failed: %s\n", snd_strerror(err));
 
    free(floatData);
    free(byteData);
    snd_pcm_close(handle);
    return 0;
}
 