# ===================================
# $Id: Makefile.entrez2client.app 25552 2003-07-31 22:59:33Z ucko $
#
# Meta-makefile for entrez2 command-line test app
# Author: Mike DiCuccio
# ===================================

APP = entrez2client
SRC = entrez2client
LIB = entrez2 entrez2cli xconnect xser xutil xncbi
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

REQUIRES = objects

