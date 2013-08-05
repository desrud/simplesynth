/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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

//#include <gtkmm.h>
#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "config.h"

#include "dssi.h"
#include "ladspa.h"


#include "WaveTable.h"


class SimpleSynth
{
public:
    static const DSSI_Descriptor *getDescriptor(unsigned long index);

private:
    SimpleSynth(int sampleRate);
    ~SimpleSynth();

    enum {
		OutputPort = 0,
		PortCount  = 1
    };

    enum {
		Notes = 128
    };

    static const char *const portNames[PortCount];
    static const LADSPA_PortDescriptor ports[PortCount];
    static const LADSPA_PortRangeHint hints[PortCount];
    static const LADSPA_Properties properties;
    static const LADSPA_Descriptor ladspaDescriptor;
    static const DSSI_Descriptor dssiDescriptor;

    static LADSPA_Handle instantiate(const LADSPA_Descriptor *, unsigned long);
    static void connectPort(LADSPA_Handle, unsigned long, LADSPA_Data *);
    static void activate(LADSPA_Handle);
    static void run(LADSPA_Handle, unsigned long);
    static void deactivate(LADSPA_Handle);
    static void cleanup(LADSPA_Handle);
    static const DSSI_Program_Descriptor *getProgram(LADSPA_Handle, unsigned long);
    static void selectProgram(LADSPA_Handle, unsigned long, unsigned long);
    static int getMidiController(LADSPA_Handle, unsigned long);
    static void runSynth(LADSPA_Handle, unsigned long,
			 snd_seq_event_t *, unsigned long);

    void runImpl(unsigned long, snd_seq_event_t *, unsigned long);
    void addSamples(int, unsigned long, unsigned long);

	float *m_output;

	WaveTable waveTable;
	
    int    m_sampleRate;
    long   m_blockStart;

	float  m_phases[Notes];
	long   m_ons[Notes];
    long   m_offs[Notes];
    int    m_velocities[Notes];
	float  m_frequencies[Notes];
};

const char *const
SimpleSynth::portNames[PortCount] =
{
    "Output",
};

const LADSPA_PortDescriptor 
SimpleSynth::ports[PortCount] =
{
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
};

const LADSPA_PortRangeHint 
SimpleSynth::hints[PortCount] =
{
    { 0, 0, 0 },
};

const LADSPA_Properties
SimpleSynth::properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;

const LADSPA_Descriptor 
SimpleSynth::ladspaDescriptor =
{
    3501, // "Unique" ID
    "simplesynth", // Label
    properties,
    "Simple Synth", // Name
    "Christian Trenner", // Maker
    "Christian Trenner", // Copyright
    PortCount,
    ports,
    portNames,
    hints,
    0, // Implementation data
    instantiate,
    connectPort,
    activate,
    run,
    0, // Run adding
    0, // Set run adding gain
    deactivate,
    cleanup
};

const DSSI_Descriptor 
SimpleSynth::dssiDescriptor =
{
    1, // DSSI API version
    &ladspaDescriptor,
    0, // Configure
    0, // Get Program
    0, // Select Program
    getMidiController,
    runSynth,
    0, // Run synth adding
    0, // Run multiple synths
    0, // Run multiple synths adding
};

const DSSI_Descriptor *
SimpleSynth::getDescriptor(unsigned long index)
{
    if (index == 0) return &dssiDescriptor;
    return 0;
}

SimpleSynth::SimpleSynth(int sampleRate) :
    m_output(0),
    m_sampleRate(sampleRate),
    m_blockStart(0)
{
    for (int i = 0; i < Notes; ++i) {
		float frequency = 440.0f * powf(2.0, (i - 69.0) / 12.0);
		m_frequencies[i] = frequency;
    }
}

SimpleSynth::~SimpleSynth()
{
}
    
LADSPA_Handle
SimpleSynth::instantiate(const LADSPA_Descriptor *, unsigned long rate)
{
    SimpleSynth *simpleSynth = new SimpleSynth(rate);
    return simpleSynth;
}

void
SimpleSynth::connectPort(LADSPA_Handle handle,
		      unsigned long port, LADSPA_Data *location)
{
    SimpleSynth *simpleSynth = (SimpleSynth *)handle;

    float **ports[PortCount] = {
		&simpleSynth->m_output,
    };

    *ports[port] = (float *)location;
}

void
SimpleSynth::activate(LADSPA_Handle handle)
{
    SimpleSynth *simpleSynth = (SimpleSynth *)handle;

    simpleSynth->m_blockStart = 0;

    for (size_t i = 0; i < Notes; ++i) {
		simpleSynth->m_ons[i] = -1;
		simpleSynth->m_offs[i] = -1;
		simpleSynth->m_velocities[i] = 0;
		simpleSynth->m_phases[i] = 0;
	}
}

void
SimpleSynth::run(LADSPA_Handle handle, unsigned long samples)
{
    runSynth(handle, samples, 0, 0);
}

void
SimpleSynth::deactivate(LADSPA_Handle handle)
{
    activate(handle); // both functions just reset the plugin
}

void
SimpleSynth::cleanup(LADSPA_Handle handle)
{
    delete (SimpleSynth *)handle;
}

int
SimpleSynth::getMidiController(LADSPA_Handle, unsigned long port)
{
    int controllers[PortCount] = {
	DSSI_NONE,
    };

    return controllers[port];
}

void
SimpleSynth::runSynth(LADSPA_Handle handle, unsigned long samples,
		       snd_seq_event_t *events, unsigned long eventCount)
{
    SimpleSynth *simpleSynth = (SimpleSynth *)handle;

    simpleSynth->runImpl(samples, events, eventCount);
}

void
SimpleSynth::runImpl(unsigned long sampleCount,
		  snd_seq_event_t *events,
		  unsigned long eventCount)
{
    unsigned long pos;
    unsigned long count;
    unsigned long eventPos;
    snd_seq_ev_note_t n;
    int i;

    for (pos = 0, eventPos = 0; pos < sampleCount; ) {

		while (eventPos < eventCount &&
			   pos >= events[eventPos].time.tick) {

			switch (events[eventPos].type) {

				case SND_SEQ_EVENT_NOTEON:
					n = events[eventPos].data.note;
					if (n.velocity > 0) {
						m_ons[n.note] = m_blockStart + events[eventPos].time.tick;
						m_offs[n.note] = -1;
						m_velocities[n.note] = n.velocity;
					}
				break;

				case SND_SEQ_EVENT_NOTEOFF:
					n = events[eventPos].data.note;
					m_offs[n.note] = m_blockStart + events[eventPos].time.tick;
					break;

					default:
				break;
			}

			++eventPos;
		}

		count = sampleCount - pos;
		if (eventPos < eventCount &&
			events[eventPos].time.tick < sampleCount) {
			count = events[eventPos].time.tick - pos;
		}

		for (i = 0; i < count; ++i) {
			m_output[pos + i] = 0;
		}

		for (i = 0; i < Notes; ++i) {
			if (m_ons[i] >= 0) {
				addSamples(i, pos, count);
			}
		}

		pos += count;
    }

    m_blockStart += sampleCount;
}

void
SimpleSynth::addSamples(int voice, unsigned long offset, unsigned long count)
{
    if (m_ons[voice] < 0) return;

    unsigned long on = (unsigned long)(m_ons[voice]);
    unsigned long start = m_blockStart + offset;

    if (start < on) return;

    if (start == on) { 
		
    }

    size_t i, s;

    float vgain = (float)(m_velocities[voice]) / 127.0f;
	float phase_increment = m_frequencies[voice] / m_sampleRate;
	
    for (i = 0, s = start - on; i < count; ++i, ++s) {

		float gain(vgain);

		//TODO looks like an envelope - only release
		if (m_offs[voice] >= 0 && (unsigned long)(m_offs[voice]) < i + start) {
			
			unsigned long release = 1 + (0.01 * m_sampleRate);
			unsigned long dist = i + start - m_offs[voice];

			if (dist > release) {
				m_ons[voice] = -1;
				break;
			}

			gain = gain * float(release - dist) / float(release);
		}

		m_phases[voice] += phase_increment;
		if (m_phases[voice] > 1.0f)
			m_phases[voice] -= (int) m_phases[voice];

		
		m_output[offset + i] += gain * waveTable.calculate(m_phases[voice]);
	}
}

extern "C" {

const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    return 0;
}

const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    return SimpleSynth::getDescriptor(index);
}

}
