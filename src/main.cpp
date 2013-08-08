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

#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "config.h"

#include "dssi.h"
#include "ladspa.h"

#include "Global.h"
#include "WaveTable.h"
#include "Voice.h"

const int numWaveTables = 5;

class SimpleSynth
{
public:
    static const DSSI_Descriptor *getDescriptor(unsigned long index);

private:
    SimpleSynth(int sampleRate);
    ~SimpleSynth();

    enum {
        OutputPort = 0,
        WaveFormSelect = 1,
        Detune = 2,
        PortCount  = 3
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
    void addSamples(float*, int, unsigned long, unsigned long);

    float *m_output;

    Settings *m_settings;

    Voice  m_voices[Notes];
};



const char *const
SimpleSynth::portNames[PortCount] =
{
    "Output",
    "WaveForm",
    "Detune",
};

const LADSPA_PortDescriptor 
SimpleSynth::ports[PortCount] =
{
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,     //Output
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,   //WaveFormSelect
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,   //Detune
};

const LADSPA_PortRangeHint 
SimpleSynth::hints[PortCount] =
{
    { 0, 0, 0 },                                                     //Output
    { LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | 
      LADSPA_HINT_INTEGER | LADSPA_HINT_BOUNDED_ABOVE, 0, 100 },     //WaveFormSelect
    { LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_BOUNDED_BELOW | 
      LADSPA_HINT_INTEGER | LADSPA_HINT_BOUNDED_ABOVE, 0, 100 },     //Detune
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
    m_output(0)
{
    m_settings = new Settings();
    m_settings->m_sampleRate = sampleRate;
    m_settings->m_detune = 0;
    m_settings->m_blockStart = 0;
    m_settings->m_waveTables = new WaveTable*[numWaveTables];
    m_settings->m_waveTables[0] = new WaveTable(sine_data, sine_size);
    m_settings->m_waveTables[1] = new WaveTable(saw_data, saw_size);
    m_settings->m_waveTables[2] = new WaveTable(square_data, square_size);
    m_settings->m_waveTables[3] = new WaveTable(pulse15_data, pulse15_size);
    m_settings->m_waveTables[4] = new WaveTable(pulse5_data, pulse5_size);

    for (int i = 0; i < Notes; i++) {
        m_voices[i].setSettings(m_settings);
    }
}

SimpleSynth::~SimpleSynth()
{
    for (int i = 0; i < numWaveTables; i++) {
        delete m_settings->m_waveTables[i];
    }

    delete[] m_settings->m_waveTables;
    delete m_settings;
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
        &simpleSynth->m_output,                //Output
        &simpleSynth->m_settings->m_waveForm,  //WaveFormSelect
        &simpleSynth->m_settings->m_detune,    //Detune
    };

    *ports[port] = (float *)location;
}

void
SimpleSynth::activate(LADSPA_Handle handle)
{
    SimpleSynth *simpleSynth = (SimpleSynth *)handle;

    simpleSynth->m_settings->m_blockStart = 0;

    for (size_t i = 0; i < Notes; ++i) {
        simpleSynth->m_voices[i].reset();
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
        DSSI_CC(69),
        DSSI_CC(70)
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
                        m_voices[n.note].noteOn(m_settings->m_blockStart + events[eventPos].time.tick, n.velocity, n.note);
                    }
                break;

                case SND_SEQ_EVENT_NOTEOFF:
                    n = events[eventPos].data.note;
                    m_voices[n.note].off = m_settings->m_blockStart + events[eventPos].time.tick;//TODO don't access off directly
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
            m_voices[i].addSamples(m_output, pos, count);
        }

        pos += count;
    }

    m_settings->m_blockStart += sampleCount;
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
