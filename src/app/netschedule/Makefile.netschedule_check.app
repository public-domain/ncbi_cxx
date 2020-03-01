#################################
# $Id: Makefile.netschedule_check.app 110132 2007-09-07 18:58:43Z didenko $
#################################

APP = netschedule_check
SRC = netschedule_check

LIB = xconnserv xthrserv xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

#CHECK_CMD  = netschedule_check.sh
#CHECK_COPY = netschedule_check.sh
