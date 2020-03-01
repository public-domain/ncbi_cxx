#################################
# $Id: Makefile.asn2flat.app 118708 2008-01-31 17:58:33Z ucko $
# Author:  Mati Shomrat
#################################

# Build application "asn2flat"
#################################

APP = asn2flat
SRC = asn2flat
LIB = xformat xobjutil submit gbseq xalnmgr xcleanup entrez2cli entrez2 tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin

