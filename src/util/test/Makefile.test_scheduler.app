# $Id: Makefile.test_scheduler.app 159017 2009-04-30 18:28:43Z ucko $

APP = test_scheduler
SRC = test_scheduler

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xutil test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =

CHECK_AUTHORS = ivanovp
