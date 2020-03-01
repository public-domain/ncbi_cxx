# $Id: Makefile.test_ncbithr.app 102985 2007-04-26 13:34:19Z ucko $

APP = test_ncbithr
SRC = test_ncbithr
LIB = xncbi

REQUIRES = MT

CHECK_CMD = test_ncbithr
CHECK_CMD = test_ncbithr -favorwriters
