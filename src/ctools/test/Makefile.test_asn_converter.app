# $Id: Makefile.test_asn_converter.app 91324 2006-10-06 14:49:52Z ivanov $

REQUIRES = objects

APP = test_asn_converter
SRC = test_asn_converter

CPPFLAGS = $(ORIG_CPPFLAGS) $(NCBI_C_INCLUDE)

LIB  = seqset $(SEQ_LIBS) pub medline biblio general xser xxconnect xutil xncbi
NCBI_C_LIBS = -lncbiobj $(NCBI_C_ncbi)
LIBS = $(NCBI_C_LIBPATH) $(NCBI_C_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)