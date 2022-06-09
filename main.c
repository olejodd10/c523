/*
 *  This small demo sends a simple sinusoidal wave to your speakers.
 */

// Almost exclusively taken from https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_8c-example.html

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
#include "stdin_monitor.h"
 
static char *device = "default";         /* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;    /* sample format */ // CPU endian! 
static unsigned int rate = 44100;           /* stream rate */
static unsigned int channels = 1;           /* count of channels */
static unsigned int buffer_time = 100000;       /* ring buffer length in us */
static unsigned int period_time = 50000;       /* period time in us */
static int verbose = 0;                 /* verbose flag */
static int resample = 1;                /* enable alsa-lib resampling */
static int period_event = 0;                /* produce poll event after each period */
  
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
static snd_output_t *output = NULL;

static int fade_length = 700;

static int set_hwparams(snd_pcm_t *handle,
            snd_pcm_hw_params_t *params,
            snd_pcm_access_t access)
{
    unsigned int rrate;
    snd_pcm_uframes_t size;
    int err, dir;
 
    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
    }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
    if (err < 0) {
        printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0) {
        printf("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) {
        printf("Channels count (%u) not available for playbacks: %s\n", channels, snd_strerror(err));
        return err;
    }
    /* set the stream rate */
    rrate = rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) {
        printf("Rate %uHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
    }
    if (rrate != rate) {
        printf("Rate doesn't match (requested %uHz, get %iHz)\n", rate, err);
        return -EINVAL;
    }
    /* set the buffer time */
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
    if (err < 0) {
        printf("Unable to set buffer time %u for playback: %s\n", buffer_time, snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0) {
        printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
        return err;
    }
    buffer_size = size;
    /* set the period time */
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
    if (err < 0) {
        printf("Unable to set period time %u for playback: %s\n", period_time, snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
    if (err < 0) {
        printf("Unable to get period size for playback: %s\n", snd_strerror(err));
        return err;
    }
    period_size = size;
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
 
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int err;
 
    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
    if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* allow the transfer when at least period_size samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
    if (err < 0) {
        printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* enable period events when requested */
    if (period_event) {
        err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
        if (err < 0) {
            printf("Unable to set period event: %s\n", snd_strerror(err));
            return err;
        }
    }
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
 
/*
 *   Underrun and suspend recovery
 */
 
static int xrun_recovery(snd_pcm_t *handle, int err)
{
    if (verbose)
        printf("stream recovery\n");
    if (err == -EPIPE) {    /* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
    } else if (err == -ESTRPIPE) {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(1);   /* wait until the suspend flag is released */
        if (err < 0) {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    }
    return err;
}

 
static int write_loop(snd_pcm_t *handle,
              signed short *samples,
              snd_pcm_channel_area_t *areas)
{
    signed short *ptr;
    int err, nSamples;

    SineWave waves[NUM_NOTES];
    for (Note i = 0; i < NUM_NOTES; ++i) {
        waves[i] = (SineWave){frequency((Note)i), 1.0, 0.0};
    }

    int noteActive[NUM_NOTES];
    for (Note i = 0; i < NUM_NOTES; ++i) {
        noteActive[i] = 0;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, monitorStdin, NULL);

    double* floatData = calloc(period_size, sizeof(double));
    for (int i = 0; i < period_size; ++i) {
        floatData[i] = 0.;
    }
    
    int16_t* byteData = calloc(period_size, sizeof(int16_t));
    for (int i = 0; i < period_size; ++i) {
        byteData[i] = 0; // ikke egentlig nødvendig
    }

    while (1) {
        
        // Prepare new period
        for (int i = 0; i < period_size; ++i) {
            floatData[i] = 0.;
        }

        // Get input, layer active waves and update their phase
        for (int i = 0; i < NUM_NOTES; ++i) {
            int nowActive = isActive((Note)i);
            if (nowActive) {
                if (noteActive[i]) { // Note is sustained
                    layer(waves[i], floatData, rate, period_size);
                } else { // Note is introduced
                    layerLinearFadeIn(waves[i], floatData, rate, period_size, fade_length);
                }
                shiftPhase(&waves[i], rate, period_size);
            } else if (noteActive[i]) { // Note is released
                layerLinearFadeOut(waves[i], floatData, rate, period_size, fade_length);
                waves[i].phase = 0.;
            }
            noteActive[i] = nowActive;
        }

        // Prepare signal for sending
        double maxAmp = maxAmplitude(floatData, period_size);
        if (maxAmp > 1.0) { // Can something prettier be done?
            scale(floatData, period_size, 1/maxAmp);
            lpcmS16(floatData, byteData, period_size, maxAmp);
        } else {
            lpcmS16(floatData, byteData, period_size, 1.0);
        }


        ptr = byteData;
        nSamples = period_size;
        while (nSamples > 0) {
            err = snd_pcm_writei(handle, ptr, nSamples);
            if (err == -EAGAIN)
                continue;
            if (err < 0) {
                if (xrun_recovery(handle, err) < 0) {
                    printf("Write error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                }
                break;  /* skip one period */
            }
            // ptr += err * channels;
            ptr += err;
            nSamples -= err;
        }
    }

    free(floatData);
    free(byteData);
}
 
 
int main()
{

    snd_pcm_t *handle;
    int err, morehelp;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
    signed short *samples;
    unsigned int chn;
    snd_pcm_channel_area_t *areas;
 
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);
 
    err = snd_output_stdio_attach(&output, stdout, 0);
    if (err < 0) {
        printf("Output failed: %s\n", snd_strerror(err));
        return 0;
    }

    printf("Playback device is %s\n", device);
    printf("Stream parameters are %uHz, %s, %u channels\n", rate, snd_pcm_format_name(format), channels);
 
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) { // Når mode her er nonblock ser jeg egentlig ikke fordel med polling framfor write_loop...
        printf("Playback open error: %s\n", snd_strerror(err));
        return 0;
    }
    
    if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) { // Både write og poll&write har INTERLEAVED
        printf("Setting of hwparams failed: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = set_swparams(handle, swparams)) < 0) {
        printf("Setting of swparams failed: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
 
    if (verbose > 0)
        snd_pcm_dump(handle, output);
 
    samples = malloc((period_size * channels * snd_pcm_format_physical_width(format)) / 8);
    if (samples == NULL) {
        printf("No enough memory\n");
        exit(EXIT_FAILURE);
    }
    
    areas = calloc(channels, sizeof(snd_pcm_channel_area_t));
    if (areas == NULL) {
        printf("No enough memory\n");
        exit(EXIT_FAILURE);
    }
    for (chn = 0; chn < channels; chn++) {
        areas[chn].addr = samples;
        areas[chn].first = chn * snd_pcm_format_physical_width(format);
        areas[chn].step = channels * snd_pcm_format_physical_width(format);
    }
 
    err = write_loop(handle, samples, areas);
    if (err < 0)
        printf("Transfer failed: %s\n", snd_strerror(err));
 
    free(areas);
    free(samples);
    snd_pcm_close(handle);
    return 0;
}
 