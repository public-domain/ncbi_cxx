# $Id: Makefile.test_qparse.app 96660 2007-01-10 17:06:31Z kuznets $

APP = test_qparse
SRC = test_qparse

CPPFLAGS = $(ORIG_CPPFLAGS) 

LIB  = xncbi xqueryparse
LIBS = $(DL_LIBS) $(ORIG_LIBS)

