# $Id: Makefile.test_threaded_server.app 27628 2003-09-22 20:33:02Z ucko $

APP = test_threaded_server
SRC = test_threaded_server
LIB = xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify -best-effort CC

REQUIRES = MT

# Tested along with test_threaded_client from its makefile.
