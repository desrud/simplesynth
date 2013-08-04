AC_INIT([SimpleSynth], [0.1], [nomail@example.com], [SimpleSynth])
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([1.10 no-define])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)
AC_PROG_LIBTOOL
AC_PROG_LN_S


dnl Check for LADSPA
AC_CHECK_HEADERS(ladspa.h)

dnl Require ALSA or libdssialsacompat
PKG_CHECK_MODULES(GTKMM,gtkmm-3.0)
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

LT_INIT

AC_OUTPUT([
    Makefile
    src/Makefile
])
