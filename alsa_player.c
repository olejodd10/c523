#include "alsa_player.h"

// Almost exclusively taken from https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_8c-example.html
// Changes made: Removed unnecessary configuration. Using buffer and period sizes instead of times

int setHWParams(snd_pcm_t *handle,
            snd_pcm_hw_params_t *params,
            snd_pcm_access_t access)
{
    unsigned int rrate;
    int err;
 
    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0) {
        printf("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, FORMAT);
    if (err < 0) {
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
    if (err < 0) {
        printf("Channels count (%u) not available for playbacks: %s\n", CHANNELS, snd_strerror(err));
        return err;
    }
    /* set the stream rate */
    rrate = STREAM_RATE;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) {
        printf("Rate %uHz not available for playback: %s\n", STREAM_RATE, snd_strerror(err));
        return err;
    }
    if (rrate != STREAM_RATE) {
        printf("Rate doesn't match (requested %uHz, get %iHz)\n", STREAM_RATE, err);
        return -EINVAL;
    }
    /* set the buffer time */
    err = snd_pcm_hw_params_set_buffer_size(handle, params, STREAM_BUFFER_LEN);
    if (err < 0) {
        printf("Unable to set buffer size %u for playback: %s\n", STREAM_BUFFER_LEN, snd_strerror(err));
        return err;
    }
    /* set the period time */
    err = snd_pcm_hw_params_set_period_size(handle, params, PERIOD_LEN, 0);
    if (err < 0) {
        printf("Unable to set period size %u for playback: %s\n", PERIOD_LEN, snd_strerror(err));
        return err;
    }
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
 
int setSWParams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
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
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (STREAM_BUFFER_LEN / PERIOD_LEN) * PERIOD_LEN);
    if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* allow the transfer when at least PERIOD_LEN samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, PERIOD_LEN);
    if (err < 0) {
        printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
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
 
static int xrunRecovery(snd_pcm_t *handle, int err)
{
    if (VERBOSE)
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


int streamNotes(snd_pcm_t *handle,
                double* floatData,
                int16_t* byteData,
              int (*isActive)(Note))
{
    int16_t *ptr;
    int err, nSamples;

    int noteActive[NUM_NOTES];
    for (Note i = 0; i < NUM_NOTES; ++i) {
        noteActive[i] = 0;
    }

    SineWave waves[NUM_NOTES];
    for (Note i = 0; i < NUM_NOTES; ++i) {
        waves[i] = (SineWave){frequency((Note)i), 1.0, 0.0};
    }

    while (1) {
        
        // Prepare new period
        for (int i = 0; i < PERIOD_LEN; ++i) {
            floatData[i] = 0.;
        }

        // Get input, layer active waves and update their phase
        for (int i = 0; i < NUM_NOTES; ++i) {
            int nowActive = isActive((Note)i);
            if (nowActive) {
                if (noteActive[i]) { // Note is sustained
                    layer(waves[i], floatData, STREAM_RATE, PERIOD_LEN);
                } else { // Note is introduced
                    layerLinearFadeIn(waves[i], floatData, STREAM_RATE, PERIOD_LEN, FADE_LEN);
                }
                shiftPhase(&waves[i], STREAM_RATE, PERIOD_LEN);
            } else if (noteActive[i]) { // Note is released
                layerLinearFadeOut(waves[i], floatData, STREAM_RATE, PERIOD_LEN, FADE_LEN);
                waves[i].phase = 0.;
            }
            noteActive[i] = nowActive;
        }

        // Prepare signal for sending
        double maxAmp = maxAmplitude(floatData, PERIOD_LEN);
        if (maxAmp > 1.0) { // Can something prettier be done?
            scale(floatData, PERIOD_LEN, 1/maxAmp);
            lpcmS16(floatData, byteData, PERIOD_LEN, maxAmp);
        } else {
            lpcmS16(floatData, byteData, PERIOD_LEN, 1.0);
        }

        // Write bytes to buffer
        ptr = byteData;
        nSamples = PERIOD_LEN;
        while (nSamples > 0) {
            err = snd_pcm_writei(handle, ptr, nSamples);
            if (err == -EAGAIN)
                continue;
            if (err < 0) {
                if (xrunRecovery(handle, err) < 0) {
                    printf("Write error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                }
                break;  /* skip one period */
            }
            ptr += err;
            nSamples -= err;
        }
    }
}
 