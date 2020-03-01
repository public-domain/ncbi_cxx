# $Id: Makefile.test_ncbi_socket.app 101876 2007-04-10 20:03:58Z ucko $

APP = test_ncbi_socket
SRC = test_ncbi_socket
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_socket.sh
CHECK_COPY = test_ncbi_socket.sh
