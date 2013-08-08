plugin_LTLIBRARIES = simplesynth.la

plugindir = $(libdir)/dssi


simplesynth_la_SOURCES = \
	Global.h \
	Voice.cpp \
	main.cpp \
	WaveTable.cpp \
	../dssi/dssi.h


simplesynth_la_CXXFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS)
simplesynth_la_LDFLAGS = -module -avoid-version

if HAVE_LIBLO
if HAVE_QT
lts_ui_PROGRAMS = simplesynth_qt
else
lts_ui_PROGRAMS =
endif
else
lts_ui_PROGRAMS =
endif

lts_uidir = $(libdir)/dssi/simplesynth

SIMPLESYNTH_MOC = SynthGUI.moc.cpp

simplesynth_qt_SOURCES = \
	SynthGUI.cpp\
	SynthGUI.h

nodist_simplesynth_qt_SOURCES = $(SIMPLESYNTH_MOC)

simplesynth_qt_CXXFLAGS = $(AM_CXXFLAGS) $(QT_CFLAGS) $(LIBLO_CFLAGS)
simplesynth_qt_LDADD = $(AM_LDFLAGS) $(QT_LIBS) $(LIBLO_LIBS)

%.moc.cpp: %.h
	$(QT_MOC) -o $@ $<

if HAVE_QT
BUILT_SOURCES = $(SIMPLESYNTH_MOC)
else
BUILT_SOURCES =
endif

CLEANFILES = $(BUILT_SOURCES)


