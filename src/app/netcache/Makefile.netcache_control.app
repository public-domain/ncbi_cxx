#################################
# $Id: Makefile.netcache_control.app 86178 2006-07-17 14:42:03Z ucko $
#################################

APP = netcache_control
SRC = netcache_control

LIB = xconnserv xthrserv xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
