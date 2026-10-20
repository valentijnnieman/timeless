/*==============================================================================
Gain DSP Plugin Example
Copyright (c), Firelight Technologies Pty, Ltd 2004-2026.

This example shows how to create a simple gain DSP effect.
==============================================================================*/

#ifdef WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "fmod.hpp"

extern "C" {
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

const float FMOD_GAIN_PARAM_GAIN_MIN     = -80.0f;
const float FMOD_GAIN_PARAM_GAIN_MAX     = 10.0f;
const float FMOD_GAIN_PARAM_GAIN_DEFAULT = 0.0f;
#define FMOD_GAIN_RAMPCOUNT 256

enum
{
    FMOD_GAIN_PARAM_GAIN = 0,
    FMOD_GAIN_PARAM_INVERT,
    FMOD_GAIN_NUM_PARAMETERS
};

#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= FMOD_GAIN_PARAM_GAIN_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? FMOD_GAIN_PARAM_GAIN_MIN : 20.0f * log10f((float)__linval__))

FMOD_RESULT F_CALL FMOD_Gain_dspcreate       (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALL FMOD_Gain_dsprelease      (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALL FMOD_Gain_dspreset        (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALL FMOD_Gain_dspprocess      (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER *inbuffer, const FMOD_DSP_BUFFER *outbuffer);
FMOD_RESULT F_CALL FMOD_Gain_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT F_CALL FMOD_Gain_dspsetparambool (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT F_CALL FMOD_Gain_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT F_CALL FMOD_Gain_dspgetparambool (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr);
FMOD_RESULT F_CALL FMOD_Gain_sys_register    (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALL FMOD_Gain_sys_deregister  (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALL FMOD_Gain_sys_mix         (FMOD_DSP_STATE *dsp_state, int stage);

static bool                    FMOD_Gain_Running = false;

extern "C"
{

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
{
    static float gain_mapping_values[] = { -80, -50, -30, -10, 10 };
    static float gain_mapping_scale[] = { 0, 2, 4, 7, 11 };

    // Initialize float parameter using a differernt mapping to create a non linear envelope. (Piecewise linear mode).
    FMOD_DSP_DECLARE_PARAMDESC_FLOAT_PIECEWISE_LINEAR(p_gain, "Gain", "Gain in dB. -80 to 10. Default = 0", "dB", FMOD_GAIN_PARAM_GAIN_MIN, FMOD_GAIN_PARAM_GAIN_MAX, FMOD_GAIN_PARAM_GAIN_DEFAULT, gain_mapping_values, gain_mapping_scale);
    FMOD_DSP_DECLARE_PARAMDESC_BOOL(p_invert, "Invert", "Invert signal. Default = off", nullptr, false);
   
    static FMOD_DSP_PARAMETER_DESC *FMOD_Gain_dspparam[FMOD_GAIN_NUM_PARAMETERS] =
    {
        &p_gain,
        &p_invert
    };

    static FMOD_DSP_DESCRIPTION desc = { FMOD_PLUGIN_SDK_VERSION, "FMOD Gain", 0x00010000, FMOD_DSP_MODE_FILTER };
    desc.create             = FMOD_Gain_dspcreate;
    desc.release            = FMOD_Gain_dsprelease;
    desc.reset              = FMOD_Gain_dspreset;
    desc.process            = FMOD_Gain_dspprocess;
    desc.numparameters      = FMOD_GAIN_NUM_PARAMETERS;
    desc.paramdesc          = FMOD_Gain_dspparam;
    desc.setparameterfloat  = FMOD_Gain_dspsetparamfloat;
    desc.setparameterbool   = FMOD_Gain_dspsetparambool;
    desc.getparameterfloat  = FMOD_Gain_dspgetparamfloat;
    desc.getparameterbool   = FMOD_Gain_dspgetparambool;
    desc.sys_register       = FMOD_Gain_sys_register;
    desc.sys_deregister     = FMOD_Gain_sys_deregister;
    desc.sys_mix            = FMOD_Gain_sys_mix;

    return &desc;
}

}

class FMODGainState
{
public:
    FMODGainState();

    void read(float *inbuffer, float *outbuffer, unsigned int length, int channels);
    void reset();
    void setGain(float);
    void setInvert(bool);
    float gain() const { return LINEAR_TO_DECIBELS(m_invert ? -m_target_gain : m_target_gain); }
    FMOD_BOOL invert() const { return m_invert; }

private:
    float m_target_gain;
    float m_current_gain;
    int   m_ramp_samples_left;
    bool  m_invert;
};

FMODGainState::FMODGainState()
{
    m_target_gain = DECIBELS_TO_LINEAR(FMOD_GAIN_PARAM_GAIN_DEFAULT);
    m_invert = 0;
    reset();
}

void FMODGainState::read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    // Note: buffers are interleaved
    float gain = m_current_gain;

    if (m_ramp_samples_left)
    {
        float target = m_target_gain;
        float delta = (target - gain) / m_ramp_samples_left;
        while (length)
        {
            if (--m_ramp_samples_left)
            {
                gain += delta;
                for (int i = 0; i < channels; ++i)
                {
                    *outbuffer++ = *inbuffer++ * gain;
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
        *outbuffer++ = *inbuffer++ * gain;
    }

    m_current_gain = gain;
}

void FMODGainState::reset()
{
    m_current_gain = m_target_gain;
    m_ramp_samples_left = 0;
}

void FMODGainState::setGain(float gain)
{
    m_target_gain = m_invert ? -DECIBELS_TO_LINEAR(gain) : DECIBELS_TO_LINEAR(gain);
    m_ramp_samples_left = FMOD_GAIN_RAMPCOUNT;
}

void FMODGainState::setInvert(bool invert)
{
    if (invert != m_invert)
    {
        m_target_gain = -m_target_gain;
        m_ramp_samples_left = FMOD_GAIN_RAMPCOUNT;
    }
    m_invert = invert;
}

FMOD_RESULT F_CALL FMOD_Gain_dspcreate(FMOD_DSP_STATE *dsp_state)
{
    dsp_state->plugindata = (FMODGainState *)FMOD_DSP_ALLOC(dsp_state, sizeof(FMODGainState));
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Gain_dsprelease(FMOD_DSP_STATE *dsp_state)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Gain_dspprocess(FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER *inbuffer, const FMOD_DSP_BUFFER *outbuffer)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

    state->read(inbuffer->buffer, outbuffer->buffer, length, inbuffer->numchannels); // input and output channels count match for this effect

    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Gain_dspreset(FMOD_DSP_STATE *dsp_state)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;
    state->reset();
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Gain_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GAIN_PARAM_GAIN:
        state->setGain(value);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Gain_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GAIN_PARAM_GAIN:
        *value = state->gain();
        if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, "%.1f dB", state->gain());
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Gain_dspsetparambool(FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

    switch (index)
    {
      case FMOD_GAIN_PARAM_INVERT:
        state->setInvert(value ? true : false);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Gain_dspgetparambool(FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr)
{
    FMODGainState *state = (FMODGainState *)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GAIN_PARAM_INVERT:
        *value = state->invert();
        if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, state->invert() ? "Inverted" : "Off" );
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Gain_sys_register(FMOD_DSP_STATE * /*dsp_state*/)
{
    FMOD_Gain_Running = true;
    // called once for this type of dsp being loaded or registered (it is not per instance)
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Gain_sys_deregister(FMOD_DSP_STATE * /*dsp_state*/)
{
    FMOD_Gain_Running = false;
    // called once for this type of dsp being unloaded or de-registered (it is not per instance)
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Gain_sys_mix(FMOD_DSP_STATE * /*dsp_state*/, int /*stage*/)
{
    // stage == 0 , before all dsps are processed/mixed, this callback is called once for this type.
    // stage == 1 , after all dsps are processed/mixed, this callback is called once for this type.
    return FMOD_OK;
}
