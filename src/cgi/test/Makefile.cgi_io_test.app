# $Id: Makefile.cgi_io_test.app 100537 2007-03-19 20:47:22Z didenko $

APP = cgi_io_test
SRC = cgi_io_test

LIB = xcgi xutil xncbi

## ...or, for FastCGI:
#
# LIB = xfcgi xutil xncbi
# LIBS = $(FASTCGI_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
