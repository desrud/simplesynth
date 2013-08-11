/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cc
 * Copyright (C) 2013 desrud <ctrenner@gmx.at>
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

#include <stdlib.h>
#include <string.h>


LowPassFilter::LowPassFilter() :
    m_a1(0), m_b0(0), m_b1(0)
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
    H(S) = 1/( S + 1)
    W = tan (PI*Fc/Fs)
    N = 1/(1+W)
    B0 = W*N
    B1 = B0
    A1 = N*(W-1) 
*/
    float w = tan(M_PI * cutoff / sampleRate);
    float n = 1.0f / (1.0f + w);
    m_b0 = w * n;
    m_b1 = m_b0;
    m_a1 = n * (w - 1.0f);
}

float
LowPassFilter::calculate(float input)
{
/*
Acc = X*B0 + Z[0]*B1 - Z[1]*A1;
Z[1] = Acc;
Z[0] = X;
X is an input of filter, Acc is an output. 
*/
    float out = input * m_b0 + m_z[0] * m_b1 - m_z[1] * m_a1;
    m_z[1] = out;
    m_z[0] = input;
    return out;
}


Voice::Voice() : m_phase_osc1(0), m_phase_osc2(0), m_on(-1), off(-1), velocity(0), freq(1), m_filter()
{
}

void
Voice::reset() {
    m_phase_osc1 = 0;
    m_phase_osc2 = 0;
    m_on = -1;
    off = -1;
    velocity = 0;
    freq = 1;
}

void
Voice::setSettings(Settings *settings)
{
    this->m_settings = settings;
}

void
Voice::noteOn(long tick, int velocity, int pitch)
{
    m_on = tick;

    off = -1;
    this->velocity = velocity;
    freq = 440.0f * powf(2.0, (pitch - 69.0) / 12.0);
    //TODO reset filter ... with current impl z needs resetting
}

void
Voice::addSamples(float *buffer, unsigned long offset, unsigned long count)
{
    if (m_on < 0) return;

    //TODO better way to caluculate cutoff freq from input
    float cutoff = (*m_settings->m_cutoff) * 100.0f;
    m_filter.setup(m_settings->m_sampleRate, cutoff, *m_settings->m_q);

    float releaseSec = *m_settings->m_release;

    unsigned long on = (unsigned long)(m_on);
    unsigned long start = m_settings->m_blockStart + offset;

    if (start < on) return;

    float vgain = (float)(velocity) / 127.0f;
    float volume = *m_settings->m_volume;

    //std::cerr << "vol: " << volume << std::endl;
    vgain *= volume;

    float centFactor = powf(2.0, *(m_settings->m_detune) / 1200.0);
    float freq_detuned = freq * centFactor;
    float phase_inc1 = freq / m_settings->m_sampleRate;
    float phase_inc2 = freq_detuned / m_settings->m_sampleRate;

    for (size_t i = 0; i < count; ++i) {

        float gain(vgain);

        //TODO looks like an envelope - only release
        if (off >= 0 && (unsigned long)(off) < i + start) {

            unsigned long release = 1 + (releaseSec * m_settings->m_sampleRate);
            unsigned long dist = i + start - off;

            if (dist > release) {
                m_on = -1;
                break;
            }

            gain = gain * float(release - dist) / float(release);
        }

        m_phase_osc1 = incrementPhase(m_phase_osc1, phase_inc1);
        m_phase_osc2 = incrementPhase(m_phase_osc2, phase_inc2);

        WaveTable *waveTable = m_settings->m_waveTables[int(*m_settings->m_waveForm)];

        float tmp = (waveTable->calculate(m_phase_osc1) + waveTable->calculate(m_phase_osc2)) * 0.5f * gain;

        //TODO wire filter
        buffer[offset + i] = tmp;//m_filter.calculate(tmp);
    }
}

float
Voice::incrementPhase(float phase, float increment)
{
    float tmp = phase + increment;
    return tmp - int(tmp);
}
