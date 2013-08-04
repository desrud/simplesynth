plugin_LTLIBRARIES = simplesynth.la

plugindir = $(libdir)/dssi


simplesynth_la_SOURCES = \
	main.cpp \
	WaveTable.cpp \
	../dssi/dssi.h


simplesynth_la_CXXFLAGS = -I$(top_srcdir)/dssi $(AM_CFLAGS) $(GTKMM_CFLAGS)

simplesynth_la_LDFLAGS = -module -avoid-version

CLEANFILES = $(BUILT_SOURCES)


