#################################
# $Id: Makefile.seqvec_bench.app 118708 2008-01-31 17:58:33Z ucko $
#################################


APP = seqvec_bench
SRC = seqvec_bench
LIB = test_mt $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

