# $Id: Makefile.test_server.app 113756 2007-11-08 21:19:16Z vasilche $

APP = test_server
SRC = test_server
LIB = xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify -best-effort CC

REQUIRES = MT

CHECK_CMD =
CHECK_AUTHORS = kazimird joukovv
CHECK_TIMEOUT = 400
