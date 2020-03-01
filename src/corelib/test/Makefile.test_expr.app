# $Id: Makefile.test_expr.app 159017 2009-04-30 18:28:43Z ucko $

APP = test_expr
SRC = test_expr

LIB = test_boost$(STATIC) xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

