# autoconf
AC_PREREQ(2.59)
AC_INIT([wonderbar], [0.0.1], [damien.lespiau@gmail.com])
AC_CONFIG_MACRO_DIR([build])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_SRCDIR([src/es-main.c])
AC_CONFIG_HEADERS([config.h])

# automake
AM_INIT_AUTOMAKE([1.11 -Wall foreign no-define])
AM_SILENT_RULES([yes])

AC_PROG_LIBTOOL

# Check for programs
AC_PROG_CC

# Check for header files
AC_HEADER_STDC

AS_COMPILER_FLAGS([WARNING_CFLAGS],
		  ["-Wall -Wcast-align -Wno-uninitialized
		   -Wno-strict-aliasing -Wempty-body -Wformat -Wformat-security
		   -Winit-self -Wdeclaration-after-statement -Wvla
		   -Wpointer-arith"])

PKG_CHECK_MODULES([GLIB],    [glib-2.0 >= 2.28])
PKG_CHECK_MODULES([CLUTTER], [clutter-1.0 >= 1.10])

AC_OUTPUT([
  src/Makefile
  Makefile
])
