#ifndef ALSA_PLAYER
#define ALSA_PLAYER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include "alsa/asoundlib.h"
#include <sys/time.h>
#include <math.h>

#include <stdint.h>
#include "notes.h"
#include "lpcm.h"

#define VERBOSE 0

#define DEVICE "default"
#define STREAM_RATE 44100 
#define FORMAT SND_PCM_FORMAT_S16 // CPU endian! NOTE: Changing this also requires making lpcmS24 for example
#define CHANNELS 1 // No other options supported

#define STREAM_BUFFER_LEN 2048
#define PERIOD_LEN 512 // It seems this must be a factor of STREAM_BUFFER_LEN that is less than STREAM_BUFFER_LEN/2.
#define FADE_LEN 400

int setHWParams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
 
int setSWParams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);

int streamNotes(snd_pcm_t *handle, double* floatData, int16_t* byteData, int (*isActive)(Note));

#endif