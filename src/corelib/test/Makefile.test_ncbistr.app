# $Id: Makefile.test_ncbistr.app 159017 2009-04-30 18:28:43Z ucko $

APP = test_ncbistr
SRC = test_ncbistr
LIB = test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =
