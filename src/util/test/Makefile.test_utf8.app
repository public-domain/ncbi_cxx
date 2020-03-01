#################################
# $Id: Makefile.test_utf8.app 159017 2009-04-30 18:28:43Z ucko $

APP = test_utf8
SRC = test_utf8
LIB = xutil test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =
