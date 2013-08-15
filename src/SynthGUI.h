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

#ifndef _SIMPLE_SYNTH_QT_GUI_H_INCLUDED_
#define _SIMPLE_SYNTH_QT_GUI_H_INCLUDED_

#include <QFrame>
#include <QDial>
#include <QLabel>
#include <QLayout>

extern "C" {
#include <lo/lo.h>
}

class SynthGUI : public QFrame
{
    Q_OBJECT

public:
    SynthGUI(const char * host, const char * port,
         QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
         QByteArray exitingPath, QWidget *w = 0);
    virtual ~SynthGUI();

    bool ready() const { return m_ready; }
    void setReady(bool ready) { m_ready = ready; }

    void setHostRequestedQuit(bool r) { m_hostRequestedQuit = r; }

public slots:
    void setWaveForm(float waveForm);
    void setSemitones(float semitones);
    void setDetune(float cents);
    void setOscBalance(float oscBalance);
    void setRelease (float release);
    void setCutoff (float cutoff);
    void setResonance (float resonance);
    void setVolume (float volume);
    void aboutToQuit();

protected slots:
    void waveFormChanged(int);
    void semitonesChanged(int);
    void detuneChanged(int);
    void oscBalanceChanged(int);
    void releaseChanged(int);
    void cutoffChanged(int);
    void resonanceChanged(int);
    void volumeChanged(int);
    void oscRecv();

protected:
    QDial *newQDial( int, int, int, int );

    QLabel *m_waveFormLabel;
    QDial *m_waveForm;
    QLabel *m_semitonesLabel;
    QDial *m_semitones;
    QLabel *m_detuneLabel;
    QDial *m_detune;
    QLabel *m_oscBalanceLabel;
    QDial *m_oscBalance;
    QLabel *m_releaseLabel;
    QDial *m_release;
    QLabel *m_cutoffLabel;
    QDial *m_cutoff;
    QLabel *m_resonanceLabel;
    QDial *m_resonance;
    QLabel *m_volumeLabel;
    QDial *m_volume;

    lo_address m_host;
    QByteArray m_controlPath;
    QByteArray m_midiPath;
    QByteArray m_programPath;
    QByteArray m_exitingPath;

    bool m_suppressHostUpdate;
    bool m_hostRequestedQuit;
    bool m_ready;
};

#endif


