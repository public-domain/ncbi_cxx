# $Id: Makefile.test_value_convert.app 131415 2008-06-19 14:53:00Z ssikorsk $

APP = test_value_convert
SRC = test_value_convert
LIB = xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(BOOST_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test

CHECK_CMD =

