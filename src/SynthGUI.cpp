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

#include "SynthGUI.h"

#include <QApplication>
#include <QPushButton>
#include <QTextStream>
#include <QTimer>
#include <iostream>
#include <unistd.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>

static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
    std::cerr << "X Error: "
          << errstr << " " << err->error_code
          << "\nin major opcode:  " << err->request_code << std::endl;
    }
    return 0;
}
#endif

using std::endl;

#define SIMPLESYNTH_PORT_WAVEFORM    1
#define SIMPLESYNTH_PORT_SEMITONES   2
#define SIMPLESYNTH_PORT_DETUNE      3
#define SIMPLESYNTH_PORT_OSC_BALANCE 4
#define SIMPLESYNTH_PORT_RELEASE     5
#define SIMPLESYNTH_PORT_CUTOFF      6
#define SIMPLESYNTH_PORT_RESONANCE   7
#define SIMPLESYNTH_PORT_VOLUME      8

lo_server osc_server = 0;

static QTextStream cerr(stderr);

SynthGUI::SynthGUI(const char * host, const char * port,
           QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
           QByteArray exitingPath, QWidget *w) :
    QFrame(w),
    m_controlPath(controlPath),
    m_midiPath(midiPath),
    m_programPath(programPath),
    m_exitingPath(exitingPath),
    m_suppressHostUpdate(true),
    m_hostRequestedQuit(false),
    m_ready(false)
{
    m_host = lo_address_new(host, port);

    QGridLayout *layout = new QGridLayout(this);

    m_waveForm = newQDial(0, 16, 1, 0);//TODO hard coded!
    m_semitones = newQDial(-24, 24, 1, 0);
    m_detune  = newQDial(0, 100, 1, 0); // (Hz - 400) * 10
    m_oscBalance = newQDial(0, 100, 1, 50);
    m_release = newQDial(0, 1000, 10, 10);
    m_cutoff = newQDial(0, 100, 1, 100);
    m_resonance = newQDial(0, 100, 1, 0);
    m_volume = newQDial(0, 100, 1, 100);

    m_waveFormLabel = new QLabel(this);
    m_semitonesLabel = new QLabel(this);
    m_detuneLabel = new QLabel(this);
    m_oscBalanceLabel = new QLabel(this);
    m_releaseLabel = new QLabel(this);
    m_cutoffLabel = new QLabel(this);
    m_resonanceLabel = new QLabel(this);
    m_volumeLabel = new QLabel(this);

    int pos = 0;

    layout->addWidget(new QLabel("WaveForm", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_waveForm, 1, pos);
    layout->addWidget(m_waveFormLabel, 2, pos);
    ++pos;

    layout->addWidget(new QLabel("Semitones", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_semitones, 1, pos);
    layout->addWidget(m_semitonesLabel, 2, pos);
    ++pos;

    layout->addWidget(new QLabel("Detune", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_detune,  1, pos);
    layout->addWidget(m_detuneLabel,  2, pos, Qt::AlignCenter);
    ++pos;

    layout->addWidget(new QLabel("OscBalance", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_oscBalance, 1, pos);
    layout->addWidget(m_oscBalanceLabel, 2, pos);
    ++pos;

    layout->addWidget(new QLabel("Release", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_release, 1, pos);
    layout->addWidget(m_releaseLabel, 2, pos);
    ++pos;

    layout->addWidget(new QLabel("Cutoff", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_cutoff, 1, pos);
    layout->addWidget(m_cutoffLabel, 2, pos);
    ++pos;

    layout->addWidget(new QLabel("Q", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_resonance, 1, pos);
    layout->addWidget(m_resonanceLabel, 2, pos);
    ++pos;

    layout->addWidget(new QLabel("Volume", this), 0, pos, Qt::AlignCenter);
    layout->addWidget(m_volume, 1, pos);
    layout->addWidget(m_volumeLabel, 2, pos);
    ++pos;

    connect(m_waveForm, SIGNAL(valueChanged(int)), this, SLOT(waveFormChanged(int)));
    connect(m_semitones, SIGNAL(valueChanged(int)), this, SLOT(semitonesChanged(int)));
    connect(m_detune,  SIGNAL(valueChanged(int)), this, SLOT(detuneChanged(int)));
    connect(m_oscBalance, SIGNAL(valueChanged(int)), this, SLOT(oscBalanceChanged(int)));
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));
    connect(m_cutoff, SIGNAL(valueChanged(int)), this, SLOT(cutoffChanged(int)));
    connect(m_resonance, SIGNAL(valueChanged(int)), this, SLOT(resonanceChanged(int)));
    connect(m_volume, SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));

    // cause some initial updates
    waveFormChanged(m_waveForm->value());
    semitonesChanged(m_semitones->value());
    detuneChanged(m_detune->value());
    oscBalanceChanged(m_oscBalance->value());
    releaseChanged(m_release->value());
    cutoffChanged(m_cutoff->value());
    resonanceChanged(m_resonance->value());
    volumeChanged(m_volume->value());

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);

    m_suppressHostUpdate = false;
}

void
SynthGUI::setWaveForm(float waveForm)
{
    m_suppressHostUpdate = true;
    m_waveForm->setValue(int(waveForm));
    m_suppressHostUpdate = false;
}

void
SynthGUI::waveFormChanged(int value)
{
    float waveForm = float(value);
    m_waveFormLabel->setText(QString("%1").arg(waveForm));
    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_WAVEFORM << " waveForm " << waveForm << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_WAVEFORM, waveForm);
    }
}

void
SynthGUI::setSemitones(float semitones)
{
    m_suppressHostUpdate = true;
    m_semitones->setValue(int(semitones));
    m_suppressHostUpdate = false;
}

void
SynthGUI::semitonesChanged(int value)
{
    float semitones = float(value);
    m_semitonesLabel->setText(QString("%1").arg(semitones));
    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_SEMITONES << " semitones " << semitones << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_SEMITONES, semitones);
    }
}

void
SynthGUI::setDetune(float cent)
{
    m_suppressHostUpdate = true;
    m_detune->setValue(int(cent));
    m_suppressHostUpdate = false;
}

void
SynthGUI::detuneChanged(int value)
{
    float cent = float(value);
    m_detuneLabel->setText(QString("%1 Cent").arg(cent));

    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_DETUNE << " freq " << cent << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_DETUNE, cent);
    }
}

void
SynthGUI::setOscBalance(float oscBalance)
{
    m_suppressHostUpdate = true;
    m_oscBalance->setValue(int(oscBalance * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::oscBalanceChanged(int value)
{
    float oscBalance = float(value) / 100.0f;
    m_oscBalanceLabel->setText(QString("%1").arg(oscBalance));

    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_OSC_BALANCE << " oscBalance " << oscBalance << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_OSC_BALANCE, oscBalance);
    }
}

void
SynthGUI::setRelease(float release)
{
    m_suppressHostUpdate = true;
    m_release->setValue(int(release * 1000));
    m_suppressHostUpdate = false;
}

void
SynthGUI::releaseChanged(int value)
{
    float release = float(value) / 1000.0f;
    m_releaseLabel->setText(QString("%1 ms").arg(value));
    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_RELEASE << " release " << release << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_RELEASE, release);
    }
}

void
SynthGUI::setCutoff(float cutoff)
{
    m_suppressHostUpdate = true;
    m_cutoff->setValue(int(cutoff * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::cutoffChanged(int value)
{
    float cutoff = float(value) / 100.0f;
    m_cutoffLabel->setText(QString("%1").arg(cutoff));
    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_CUTOFF << " cutoff " << cutoff << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_CUTOFF, cutoff);
    }
}

void
SynthGUI::setResonance(float resonance)
{
    m_suppressHostUpdate = true;
    m_resonance->setValue(int(resonance * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::resonanceChanged(int value)
{
    float resonance = float(value) / 100.0f;
    m_resonanceLabel->setText(QString("%1").arg(resonance));
    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_RESONANCE << " q " << resonance << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_RESONANCE, resonance);
    }
}

void
SynthGUI::setVolume(float volume)
{
    m_suppressHostUpdate = true;
    m_volume->setValue(int(volume * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::volumeChanged(int value)
{
    float volume = float(value) / 100.0f;
    m_volumeLabel->setText(QString("%1").arg(volume));
    if (!m_suppressHostUpdate) {
        cerr << "Sending to host: " << m_controlPath
             << " port " << SIMPLESYNTH_PORT_VOLUME << " volume " << volume << endl;
        lo_send(m_host, m_controlPath, "if", SIMPLESYNTH_PORT_VOLUME, volume);
    }
}


void
SynthGUI::oscRecv()
{
    if (osc_server) {
        lo_server_recv_noblock(osc_server, 1);
    }
}

void
SynthGUI::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

SynthGUI::~SynthGUI()
{
    lo_address_free(m_host);
}

QDial *
SynthGUI::newQDial( int minValue, int maxValue, int pageStep, int value )
{
    QDial *dial = new QDial( this );
    dial->setMinimum( minValue );
    dial->setMaximum( maxValue );
    dial->setPageStep( pageStep );
    dial->setValue( value );
    dial->setNotchesVisible(true);
    return dial;
}


void
osc_error(int num, const char *msg, const char *path)
{
    cerr << "Error: liblo server error " << num
     << " in path \"" << (path ? path : "(null)")   
     << "\": " << msg << endl;
}

int
debug_handler(const char *path, const char *types, lo_arg **argv,
          int argc, void *data, void *user_data)
{
    int i;

    cerr << "Warning: unhandled OSC message in GUI:" << endl;

    for (i = 0; i < argc; ++i) {
    cerr << "arg " << i << ": type '" << types[i] << "': ";
        lo_arg_pp((lo_type)types[i], argv[i]);
    cerr << endl;
    }

    cerr << "(path is <" << path << ">)" << endl;
    return 1;
}

int
program_handler(const char *path, const char *types, lo_arg **argv,
           int argc, void *data, void *user_data)
{
    cerr << "Program handler not yet implemented" << endl;
    return 0;
}

int
configure_handler(const char *path, const char *types, lo_arg **argv,
          int argc, void *data, void *user_data)
{
    return 0;
}

int
rate_handler(const char *path, const char *types, lo_arg **argv,
         int argc, void *data, void *user_data)
{
    return 0; /* ignore it */
}

int
show_handler(const char *path, const char *types, lo_arg **argv,
         int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else gui->show();
    return 0;
}

int
hide_handler(const char *path, const char *types, lo_arg **argv,
         int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    gui->hide();
    return 0;
}

int
quit_handler(const char *path, const char *types, lo_arg **argv,
         int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int
control_handler(const char *path, const char *types, lo_arg **argv,
        int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);

    if (argc < 2) {
        cerr << "Error: too few arguments to control_handler" << endl;
        return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

    switch (port) {

    case SIMPLESYNTH_PORT_WAVEFORM:
        cerr << "gui setting waveform to " << value << endl;
        gui->setWaveForm(value);
    break;

    case SIMPLESYNTH_PORT_SEMITONES:
        cerr << "gui setting semitones to " << value << endl;
        gui->setSemitones(value);
    break;

    case SIMPLESYNTH_PORT_DETUNE:
        cerr << "gui setting detune to " << value << endl;
        gui->setDetune(value);
    break;

    case SIMPLESYNTH_PORT_OSC_BALANCE:
        cerr << "gui setting oscBalance to " << value << endl;
        gui->setOscBalance(value);
    break;

    case SIMPLESYNTH_PORT_RELEASE:
        cerr << "gui setting release to " << value << endl;
        gui->setRelease(value);
    break;

    case SIMPLESYNTH_PORT_CUTOFF:
        cerr << "gui setting cutoff to " << value << endl;
        gui->setCutoff(value);
    break;

    case SIMPLESYNTH_PORT_RESONANCE:
        cerr << "gui setting q to " << value << endl;
        gui->setResonance(value);
    break;

    case SIMPLESYNTH_PORT_VOLUME:
        cerr << "gui setting volume to " << value << endl;
        gui->setVolume(value);
    break;

    default:
        cerr << "Warning: received request to set nonexistent port " << port << " to value " << value << endl;
    }

    return 0;
}

int
main(int argc, char **argv)
{
    cerr << "SimpleSynth starting..." << endl;

    QApplication application(argc, argv);

    if (application.argc() != 5) {
    cerr << "usage: "
         << application.argv()[0] 
         << " <osc url>"
         << " <plugin dllname>"
         << " <plugin label>"
         << " <user-friendly id>"
         << endl;
    return 2;
    }

#ifdef Q_WS_X11
    XSetErrorHandler(handle_x11_error);
#endif

    char *url = application.argv()[1];

    char *host = lo_url_get_hostname(url);
    char *port = lo_url_get_port(url);
    char *path = lo_url_get_path(url);

    SynthGUI gui(host, port,
         QByteArray(path) + "/control",
         QByteArray(path) + "/midi",
         QByteArray(path) + "/program",
         QByteArray(path) + "/exiting",
         0);

    QByteArray myControlPath = QByteArray(path) + "/control";
    QByteArray myProgramPath = QByteArray(path) + "/program";
    QByteArray myConfigurePath = QByteArray(path) + "/configure";
    QByteArray myRatePath = QByteArray(path) + "/sample-rate";
    QByteArray myShowPath = QByteArray(path) + "/show";
    QByteArray myHidePath = QByteArray(path) + "/hide";
    QByteArray myQuitPath = QByteArray(path) + "/quit";

    osc_server = lo_server_new(NULL, osc_error);
    lo_server_add_method(osc_server, myControlPath, "if", control_handler, &gui);
    lo_server_add_method(osc_server, myProgramPath, "ii", program_handler, &gui);
    lo_server_add_method(osc_server, myConfigurePath, "ss", configure_handler, &gui);
    lo_server_add_method(osc_server, myRatePath, "i", rate_handler, &gui);
    lo_server_add_method(osc_server, myShowPath, "", show_handler, &gui);
    lo_server_add_method(osc_server, myHidePath, "", hide_handler, &gui);
    lo_server_add_method(osc_server, myQuitPath, "", quit_handler, &gui);
    lo_server_add_method(osc_server, NULL, NULL, debug_handler, &gui);

    lo_address hostaddr = lo_address_new(host, port);
    lo_send(hostaddr,
        QByteArray(path) + "/update",
        "s",
        (QByteArray(lo_server_get_url(osc_server))+QByteArray(path+1)).data());

    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setReady(true);
    return application.exec();
}

