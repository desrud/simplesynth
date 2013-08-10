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

#include "WaveTable.h"

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
    int idx = int(phase * m_size);
    return (m_data[idx]);//TODO some interpolation
}


