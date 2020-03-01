# $Id: Makefile.pairwise_aln.app 93543 2006-11-14 20:43:23Z todorov $

APP = pairwise_aln
SRC = pairwise_aln_app
LIB = submit $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
