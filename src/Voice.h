/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * simple-synth
 * Copyright (C) drd 2013 <drd@muh>
 * 
 * simple-synth is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * simple-synth is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SIMPLE_SYNTH_VOICE_H_
#define _SIMPLE_SYNTH_VOICE_H_

#include "Global.h"

 
const int zCount = 2;

class LowPassFilter
{
private:
    float m_b0;
    float m_b1;
    float m_a1;

    float m_z[zCount];

public:
    LowPassFilter();
    float calculate(float input);
    void reset();
    void setup(int sampleRate, float freq, float q);
};


class Voice
{
private:
    Settings *m_settings;

    long m_on;
    float m_phase_osc1;
    float m_phase_osc2;
    LowPassFilter m_filter;

    float incrementPhase(float phase, float increment);

public:

    long off;
    int velocity;
    float freq;
    
    Voice();
    void reset();

    void noteOn(long tick, int velocity, int pitch);
    void setSettings(Settings *settings);
    void addSamples(float *buffer, unsigned long offset, unsigned long count);
};

#endif // _SIMPLE_SYNTH_VOICE_H_

