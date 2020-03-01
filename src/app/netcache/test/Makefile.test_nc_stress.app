# $Id: Makefile.test_nc_stress.app 86178 2006-07-17 14:42:03Z ucko $

APP = test_nc_stress
SRC = test_nc_stress
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

