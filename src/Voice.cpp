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
#include <math.h>

#include <stdlib.h>
#include <string.h>

Voice::Voice() : m_phase_osc1(0), m_phase_osc2(0), m_on(-1), off(-1), velocity(0), freq(1)
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
}

void
Voice::addSamples(float *buffer, unsigned long offset, unsigned long count)
{
    if (m_on < 0) return;

    unsigned long on = (unsigned long)(m_on);
    unsigned long start = m_settings->m_blockStart + offset;

    if (start < on) return;

    float vgain = (float)(velocity) / 127.0f;

    float centFactor = powf(2.0, *(m_settings->m_detune) / 1200.0);
    float freq_detuned = freq * centFactor;
    float phase_inc1 = freq / m_settings->m_sampleRate;
    float phase_inc2 = freq_detuned / m_settings->m_sampleRate;

    for (size_t i = 0; i < count; ++i) {

        float gain(vgain);

        //TODO looks like an envelope - only release
        if (off >= 0 && (unsigned long)(off) < i + start) {

            unsigned long release = 1 + (0.01 * m_settings->m_sampleRate);
            unsigned long dist = i + start - off;

            if (dist > release) {
                m_on = -1;
                break;
            }

            gain = gain * float(release - dist) / float(release);
        }

        m_phase_osc1 = incrementPhase(m_phase_osc1, phase_inc1);
        m_phase_osc2 = incrementPhase(m_phase_osc2, phase_inc2);

        buffer[offset + i] += 0.5f * gain * m_settings->m_waveTable->calculate(m_phase_osc1);
        buffer[offset + i] += 0.5f * gain * m_settings->m_waveTable->calculate(m_phase_osc2);
    }
}

float
Voice::incrementPhase(float phase, float increment)
{
    float tmp = phase + increment;
    return tmp - int(tmp);
}
