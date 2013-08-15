/* -*- Mode: Cpp; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * simplesynth
 * Copyright (C) 2013 Christian Trenner <ctrenner@gmx.at>
 * 
 * simpleSynth is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * simpleSynth is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Voice.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <algorithm>

#include <stdlib.h>
#include <string.h>


LowPassFilter::LowPassFilter() :
    m_a1(0), m_a2(0), m_b0(0), m_b1(0), m_b2(0)
{
    reset();
}

void
LowPassFilter::reset()
{
    for (int i = 0; i < zCount; i++) {
        m_z[i] = 0;
    }
}

void
LowPassFilter::setup(int sampleRate, float cutoff, float q)
{
    //see http://freeverb3.sourceforge.net/iir_filter.shtml
/*
    H(S) = 1/(S^2 + S/Q + 1)
    W = tan (PI*Fc/Fs)
    N = 1/(W^2 + W/Q + 1)
    B0 = N*W^2
    B1 = 2*B0
    B2 = B0
    A1 = 2*N*(W^2 - 1)
    A2 = N*(W^2 - W/Q + 1) 
*/
    float w = tan(M_PI * cutoff / sampleRate);
    float n = 1.0f / ((w * w) + (w / q) + 1.0);
    m_b0 = (w * w) * n;
    m_b1 = 2 * m_b0;
    m_b2 = m_b0;
    m_a1 = 2.0f * n * ((w * w) - 1.0f);
    m_a2 = n * ((w * w) - (w / q) + 1.0f);
}

float
LowPassFilter::calculate(float input)
{
/*
    Acc = X*B0 + Z[0]*B1 + Z[1]*B2 - Z[2]*A1 - Z[3]*A2;
    Z[3] = Z[2];
    Z[2] = Acc;
    Z[1] = Z[0];
    Z[0] = X
    X is an input of filter, Acc is an output. 
*/
    float out = input * m_b0 + m_z[0] * m_b1 + m_z[1] * m_b2 - m_z[2] * m_a1 - m_z[3] * m_a2;
    m_z[3] = m_z[2];
    m_z[2] = out;
    m_z[1] = m_z[0];
    m_z[0] = input;
    return out;
}


Voice::Voice() : m_phase_osc1(0), m_phase_osc2(0), m_on(-1), m_off(-1), m_velocity(0), m_filter(), m_pitch(-1)
{
}

void
Voice::reset() {
    m_phase_osc1 = 0;
    m_phase_osc2 = 0;
    m_on = -1;
    m_off = -1;
    m_velocity = 0;
    m_pitch = -1;
}

void
Voice::setSettings(Settings *settings)
{
    this->m_settings = settings;
}

bool
Voice::isInUse()
{
    return m_on != -1;
}

void
Voice::noteOn(long tick, int velocity, int pitch)
{
    m_on = tick;

    m_off = -1;
    m_velocity = velocity;
    m_pitch = pitch;

    //reset filter ... with current impl z needs resetting
    m_filter.reset();
}

void
Voice::noteOff(long tick)
{
    m_off = tick;
    m_pitch = -1;
}

int
Voice::getPitch()
{
    return m_pitch;
}

void
Voice::addSamples(float *buffer, unsigned long offset, unsigned long count)
{
    if (m_on < 0) return;

    float cutoff = 50.0f * powf(1.0617f, (*m_settings->m_cutoff) * 100.0f);
    cutoff = std::min(cutoff, float(m_settings->m_sampleRate / 2 -1));

    float q = powf(1.0641, *m_settings->m_q * 100.0f);
    m_filter.setup(m_settings->m_sampleRate, cutoff, q);

    float releaseSec = *m_settings->m_release;

    unsigned long on = (unsigned long)(m_on);
    unsigned long start = m_settings->m_blockStart + offset;

    if (start < on) return;

    float vgain = (float)(m_velocity) / 127.0f;
    float volume = *m_settings->m_volume;

    //std::cerr << "vol: " << volume << std::endl;
    vgain *= volume;

    float centFactor = powf(2.0, *(m_settings->m_detune) / 1200.0);
    float freq1 = 440.0f * powf(2.0f, (m_pitch - 69.0f) / 12.0f);
    float freq2 = 440.0f * powf(2.0f, (m_pitch - 69.0f + *m_settings->m_semitones) / 12.0f) * centFactor;
    float phase_inc1 = freq1 / m_settings->m_sampleRate;
    float phase_inc2 = freq2 / m_settings->m_sampleRate;

    for (size_t i = 0; i < count; ++i) {

        float gain(vgain);

        //TODO looks like an envelope - only release
        if (m_off >= 0 && (unsigned long)(m_off) < i + start) {

            unsigned long release = 1 + (releaseSec * m_settings->m_sampleRate);
            unsigned long dist = i + start - m_off;

            if (dist > release) {
                m_on = -1;
                break;
            }

            gain = gain * float(release - dist) / float(release);
        }

        m_phase_osc1 = incrementPhase(m_phase_osc1, phase_inc1);
        m_phase_osc2 = incrementPhase(m_phase_osc2, phase_inc2);

        WaveTable *waveTable = m_settings->m_waveTables[int(*m_settings->m_waveForm)];

        float gainOsc1 = std::min(1.0f -*m_settings->m_oscBalance, 0.5f);
        float gainOsc2 = std::min(*m_settings->m_oscBalance, 0.5f);

        float tmp = (waveTable->calculate(m_phase_osc1) * gainOsc1 
                    + waveTable->calculate(m_phase_osc2) * gainOsc2) * gain;

        buffer[offset + i] += m_filter.calculate(tmp);
    }
}

float
Voice::incrementPhase(float phase, float increment)
{
    float tmp = phase + increment;
    return tmp - int(tmp);
}
