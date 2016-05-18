# 2015年 01月 29日 星期四 16:09:47 CST
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
m4_include([version.m4])
AC_INIT(zx-client,VERSION_NUMBER)
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_PROG_LIBTOOL


# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S

AC_PROG_MAKE_SET


# Checks for libraries.
PKG_CHECK_MODULES(ALLLIBS,[libxml-2.0 edk-lib tea])
CFLAGS="$CFLAGS $ALLLIBS_CFLAGS"
CXXFLAGS="$CFLAGS $ALLLIBS_CFLAGS"
LIBS="$LIBS $ALLLIBS_LIBS"

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([bzero socket])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
