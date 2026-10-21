/*==============================================================================
Plugin Example
Copyright (c), Firelight Technologies Pty, Ltd 2004-2026.

This example shows how to created a plugin effect.
==============================================================================*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fmod.hpp"

extern "C" {
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

const float FMOD_NOISE_PARAM_GAIN_MIN     = -80.0f;
const float FMOD_NOISE_PARAM_GAIN_MAX     = 10.0f;
const float FMOD_NOISE_PARAM_GAIN_DEFAULT = 0.0f;

#define FMOD_NOISE_RAMPCOUNT 256

enum
{
    FMOD_NOISE_PARAM_LEVEL = 0,
    FMOD_NOISE_PARAM_FORMAT,
    FMOD_NOISE_NUM_PARAMETERS
};

enum FMOD_NOISE_FORMAT
{
    FMOD_NOISE_FORMAT_MONO = 0,
    FMOD_NOISE_FORMAT_STEREO,
    FMOD_NOISE_FORMAT_5POINT1
};

#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= FMOD_NOISE_PARAM_GAIN_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? FMOD_NOISE_PARAM_GAIN_MIN : 20.0f * log10f((float)__linval__))

FMOD_RESULT F_CALL FMOD_Noise_dspcreate       (FMOD_DSP_STATE *dsp);
FMOD_RESULT F_CALL FMOD_Noise_dsprelease      (FMOD_DSP_STATE *dsp);
FMOD_RESULT F_CALL FMOD_Noise_dspreset        (FMOD_DSP_STATE *dsp);
FMOD_RESULT F_CALL FMOD_Noise_dspprocess      (FMOD_DSP_STATE *dsp, unsigned int length, const FMOD_DSP_BUFFER *inbuffer, const FMOD_DSP_BUFFER *outbuffer);
FMOD_RESULT F_CALL FMOD_Noise_dspsetparamfloat(FMOD_DSP_STATE *dsp, int index, float value);
FMOD_RESULT F_CALL FMOD_Noise_dspsetparamint  (FMOD_DSP_STATE *dsp, int index, int value);
FMOD_RESULT F_CALL FMOD_Noise_dspsetparambool (FMOD_DSP_STATE *dsp, int index, bool value);
FMOD_RESULT F_CALL FMOD_Noise_dspsetparamdata (FMOD_DSP_STATE *dsp, int index, void *data, unsigned int length);
FMOD_RESULT F_CALL FMOD_Noise_dspgetparamfloat(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
FMOD_RESULT F_CALL FMOD_Noise_dspgetparamint  (FMOD_DSP_STATE *dsp, int index, int *value, char *valuestr);
FMOD_RESULT F_CALL FMOD_Noise_dspgetparambool (FMOD_DSP_STATE *dsp, int index, bool *value, char *valuestr);
FMOD_RESULT F_CALL FMOD_Noise_dspgetparamdata (FMOD_DSP_STATE *dsp, int index, void **value, unsigned int *length, char *valuestr);

const char* FMOD_Noise_Format_Names[3] = {"Mono", "Stereo", "5.1"};

extern "C"
{

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
{
    FMOD_DSP_DECLARE_PARAMDESC_FLOAT(p_level, "Level", "Gain in dB. -80 to 10. Default = 0", "dB", FMOD_NOISE_PARAM_GAIN_MIN, FMOD_NOISE_PARAM_GAIN_MAX, FMOD_NOISE_PARAM_GAIN_DEFAULT);
    FMOD_DSP_DECLARE_PARAMDESC_ENUM(p_format, "Format", "Mono, stereo or 5.1. Default = 0 (mono)", FMOD_Noise_Format_Names, FMOD_NOISE_FORMAT_MONO);

    static FMOD_DSP_PARAMETER_DESC *FMOD_Noise_dspparam[FMOD_NOISE_NUM_PARAMETERS] =
    {
        &p_level,
        &p_format
    };

    static FMOD_DSP_DESCRIPTION desc = { FMOD_PLUGIN_SDK_VERSION, "FMOD Noise", 0x00010000, FMOD_DSP_MODE_GENERATOR };
    desc.create             = FMOD_Noise_dspcreate;
    desc.release            = FMOD_Noise_dsprelease;
    desc.reset              = FMOD_Noise_dspreset;
    desc.process            = FMOD_Noise_dspprocess;
    desc.numparameters      = FMOD_NOISE_NUM_PARAMETERS;
    desc.paramdesc          = FMOD_Noise_dspparam;
    desc.setparameterfloat  = FMOD_Noise_dspsetparamfloat;
    desc.setparameterint    = FMOD_Noise_dspsetparamint;
    desc.getparameterfloat  = FMOD_Noise_dspgetparamfloat;
    desc.getparameterint    = FMOD_Noise_dspgetparamint;

    return &desc;
}

}

class FMODNoiseState
{
public:
    FMODNoiseState();

    void generate(float *outbuffer, unsigned int length, int channels);
    void reset();
    void setLevel(float);
    void setFormat(FMOD_NOISE_FORMAT format) { m_format = format; }
    float level() const { return LINEAR_TO_DECIBELS(m_target_level); }
    FMOD_NOISE_FORMAT format() const { return m_format; }

private:
    float m_target_level;
    float m_current_level;
    int m_ramp_samples_left;
    FMOD_NOISE_FORMAT m_format;
};

FMODNoiseState::FMODNoiseState()
{
    m_target_level = DECIBELS_TO_LINEAR(FMOD_NOISE_PARAM_GAIN_DEFAULT);
    m_format = FMOD_NOISE_FORMAT_MONO;
    reset();
}

void FMODNoiseState::generate(float *outbuffer, unsigned int length, int channels)
{
    // Note: buffers are interleaved
    float gain = m_current_level;

    if (m_ramp_samples_left)
    {
        float target = m_target_level;
        float delta = (target - gain) / m_ramp_samples_left;
        while (length)
        {
            if (--m_ramp_samples_left)
            {
                gain += delta;
                for (int i = 0; i < channels; ++i)
                {
                    *outbuffer++ = (((float)(rand()%32768) / 16384.0f) - 1.0f) * gain;
                }
            }
            else
            {
                gain = target;
                break;
            }
            --length;
        }
    }

    unsigned int samples = length * channels;
    while (samples--)
    {
        *outbuffer++ = (((float)(rand()%32768) / 16384.0f) - 1.0f) * gain;
    }

    m_current_level = gain;
}

void FMODNoiseState::reset()
{
    m_current_level = m_target_level;
    m_ramp_samples_left = 0;
}

void FMODNoiseState::setLevel(float level)
{
    m_target_level = DECIBELS_TO_LINEAR(level);
    m_ramp_samples_left = FMOD_NOISE_RAMPCOUNT;
}

FMOD_RESULT F_CALL FMOD_Noise_dspcreate(FMOD_DSP_STATE *dsp)
{
    dsp->plugindata = (FMODNoiseState *)FMOD_DSP_ALLOC(dsp, sizeof(FMODNoiseState));
    if (!dsp->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Noise_dsprelease(FMOD_DSP_STATE *dsp)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;
    FMOD_DSP_FREE(dsp, state);
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Noise_dspprocess(FMOD_DSP_STATE *dsp, unsigned int length, const FMOD_DSP_BUFFER * /*inbuffer*/, const FMOD_DSP_BUFFER *outbuffer)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;

    state->generate(outbuffer->buffer, length, outbuffer->numchannels);
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Noise_dspreset(FMOD_DSP_STATE *dsp)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;
    state->reset();
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Noise_dspsetparamfloat(FMOD_DSP_STATE *dsp, int index, float value)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;

    switch (index)
    {
    case FMOD_NOISE_PARAM_LEVEL:
        state->setLevel(value);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Noise_dspgetparamfloat(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;

    switch (index)
    {
    case FMOD_NOISE_PARAM_LEVEL:
        *value = state->level();
        if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, "%.1f dB", state->level());
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Noise_dspsetparamint(FMOD_DSP_STATE *dsp, int index, int value)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;

    switch (index)
    {
    case FMOD_NOISE_PARAM_FORMAT:
        state->setFormat((FMOD_NOISE_FORMAT)value);

        switch(state->format())
        {
        case FMOD_NOISE_FORMAT_MONO:
            FMOD_DSP_SETOUTPUTFORMAT(dsp, 1, FMOD_SPEAKERMODE_MONO);
            break;

        case FMOD_NOISE_FORMAT_STEREO:
            FMOD_DSP_SETOUTPUTFORMAT(dsp, 2, FMOD_SPEAKERMODE_STEREO);
            break;

        case FMOD_NOISE_FORMAT_5POINT1:
            FMOD_DSP_SETOUTPUTFORMAT(dsp, 6, FMOD_SPEAKERMODE_5POINT1);
        }

        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Noise_dspgetparamint(FMOD_DSP_STATE *dsp, int index, int *value, char *valuestr)
{
    FMODNoiseState *state = (FMODNoiseState *)dsp->plugindata;

    switch (index)
    {
    case FMOD_NOISE_PARAM_FORMAT:
        *value = state->format();
        if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, "%s", FMOD_Noise_Format_Names[state->format()]);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}
