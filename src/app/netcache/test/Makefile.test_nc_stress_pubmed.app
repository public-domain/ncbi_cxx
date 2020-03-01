# $Id: Makefile.test_nc_stress_pubmed.app 143011 2008-10-14 16:37:37Z ivanovp $

APP = test_nc_stress_pubmed
SRC = test_nc_stress_pubmed
LIB = xconnserv$(STATIC) xthrserv$(STATIC) xconnect$(STATIC) xutil$(STATIC) xncbi$(STATIC)

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

