# $Id: Makefile.test_ncbi_http_get.app 143820 2008-10-22 18:54:08Z lavr $

APP = test_ncbi_http_get
SRC = test_ncbi_http_get
LIB = connect connssl $(NCBIATOMIC_LIB)

LIBS = $(GNUTLS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_http_get.sh
CHECK_COPY = test_ncbi_http_get.sh
