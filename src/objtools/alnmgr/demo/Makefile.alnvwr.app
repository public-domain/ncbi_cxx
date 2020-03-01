# $Id: Makefile.alnvwr.app 118708 2008-01-31 17:58:33Z ucko $


APP = alnvwr
SRC = alnvwrapp
LIB = xalnmgr submit tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
