# $Id: Makefile.test_ncbi_ftp_connector.app 102677 2007-04-20 19:29:48Z lavr@NCBI.NLM.NIH.GOV $

APP = test_ncbi_ftp_connector
SRC = test_ncbi_ftp_connector
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
