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

#ifndef _SIMPLE_SYNTH_GLOBAL_H_
#define _SIMPLE_SYNTH_GLOBAL_H_

#include "WaveTable.h"

struct Settings
{
    int    m_sampleRate;
    float *m_detune;
    long   m_blockStart;
    WaveTable *m_waveTable;
};

#endif // __SIMPLE_SYNTH_GLOBAL_H_
