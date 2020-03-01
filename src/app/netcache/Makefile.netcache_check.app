#################################
# $Id: Makefile.netcache_check.app 147861 2008-12-16 21:17:46Z ivanovp $
#################################

APP = netcache_check
SRC = netcache_check

LIB = xconnserv xthrserv xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD     = netcache_check.sh
CHECK_COPY    = netcache_check.sh
CHECK_TIMEOUT = 250
