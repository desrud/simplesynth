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

dnl Check for liblo
PKG_CHECK_MODULES(LIBLO, liblo >= 0.12, with_liblo=yes, with_liblo=no)
AC_SUBST(LIBLO_CFLAGS)
AC_SUBST(LIBLO_LIBS)
AM_CONDITIONAL(HAVE_LIBLO, test x$with_liblo = xyes)

dnl Check for Qt
with_qt=no
PKG_CHECK_MODULES(QT, [QtCore >= 4.0.1 QtGui >= 4.0.1],
    [with_qt=yes
     QT_MOC=`$PKG_CONFIG --variable=moc_location QtCore`],
    AC_MSG_NOTICE([pkg-config could not find Qt >= 4.0.1]))
if test "x${with_qt}" = xno ; then
  case "${host_os}" in
  darwin*)
    AC_MSG_NOTICE([checking for Qt OS X frameworks])
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    dssi_save_cppflags="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I/Library/Frameworks/QtCore.framework/Headers"
    CPPFLAGS="$CPPFLAGS -I/Library/Frameworks/QtGui.framework/Headers"
    AC_CHECK_HEADERS(QApplication, [with_qt=yes])
    CPPFLAGS="$dssi_save_cppflags"
    AC_LANG_RESTORE
    if test "x${with_qt}" = xyes ; then
      AC_PATH_PROG(QT_MOC, moc)
      if test "x$QT_MOC" = x ; then
        echo moc not found
        with_qt=no
      else
        QT_CFLAGS="-I/Library/Frameworks/QtCore.framework/Headers"
        QT_CFLAGS="$QT_CFLAGS -I/Library/Frameworks/QtGui.framework/Headers"
        QT_LIBS="-Xlinker -framework -Xlinker QtCore -Xlinker -framework -Xlinker QtGui"
      fi
    fi
    ;;
  esac
fi
if test "x${with_qt}" = xyes ; then
  dnl both less_trivial_synth_qt_gui and trivial_sampler_qt_gui need
  dnl '-lX11' in QT_LIBS when on an X11 system (because they both
  dnl utilize X11 directly; we check if Q_WS_X11 is defined.)
  AC_MSG_CHECKING([whether Qt is X11-based])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  dssi_save_cxxflags="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS $QT_CFLAGS"
  AC_TRY_COMPILE([#include <QApplication>],
      [#ifndef Q_WS_X11
       #error notX11
       #endif
      ], [QT_LIBS="$QT_LIBS -lX11"
          AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])])
  CXXFLAGS="$dssi_save_cxxflags"
  AC_LANG_RESTORE
fi
AC_SUBST(QT_CFLAGS)
AC_SUBST(QT_LIBS)
AC_SUBST(QT_MOC)
AM_CONDITIONAL(HAVE_QT, test x$with_qt = xyes)



LT_INIT

AC_OUTPUT([
    Makefile
    src/Makefile
])
