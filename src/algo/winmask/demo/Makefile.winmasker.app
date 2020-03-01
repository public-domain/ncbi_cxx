# $Id: Makefile.winmasker.app 55850 2005-02-12 19:41:01Z dicuccio $

ASN_DEP = seq

APP = winmasker
SRC = main win_mask_app
LIB = xalgowinmask \
	  xblast xnetblastcli xnetblast scoremat seqdb blastdb tables \
	  xobjread xobjutil $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

