/* -*- Mode: Cpp; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * simplesynth
 * Copyright (C) 2013 Christian Trenner <ctrenner@gmx.at>
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

#include "WaveTable.h"

const bool useInterpolation = true;

WaveTable::WaveTable(const float* data, const int size)
{
    m_size = size;
    m_data = new float[m_size + 1];

    for (int i = 0; i <= size; ++i) {
        m_data[i] = data[i];
    }
    m_data[m_size] = m_data[0];
}

WaveTable::~WaveTable() {
    delete[] m_data;
}

const float
WaveTable::calculate(const float phase)
{
    //phase is in [0, 1]

    if (useInterpolation) {
        int idx = int(phase * m_size);
        float y0 = m_data[idx];
        float y1 = m_data[idx + 1];
        float x = phase * m_size;
        float x0 = idx;

        return y0 + (y1 - y0) * (x - x0);
    } else {
        int idx = int(phase * m_size);
        return (m_data[idx]);
    }
}


