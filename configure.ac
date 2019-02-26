#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(librpc, 1.0, sunhaitao2008@hotmail)
AC_CONFIG_SRCDIR([test/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_AR

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lboost_log':
AC_CHECK_LIB([boost_log], [main])
# FIXME: Replace `main' with a function in `-lboost_serialization':
AC_CHECK_LIB([boost_serialization], [main])
# FIXME: Replace `main' with a function in `-lboost_system':
AC_CHECK_LIB([boost_system], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT([])

# enable debug or not                                                         
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],[debug program(default is no)])],
    [CXXFLAGS = "-g -DDEBUG"],
    [CXXFLAGS = "-O3"])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT
