#!/bin/sh
if [ -x "`which autoreconf 2>/dev/null`" ] ; then
   exec autoreconf -ivf
fi

LIBTOOLIZE=libtoolize
SYSNAME=`uname`
if [ "x$SYSNAME" = "xDarwin" ] ; then
  LIBTOOLIZE=glibtoolize
fi
aclocal && \
	autoheader && \
	$LIBTOOLIZE && \
	autoconf && \
	automake --add-missing --copy
