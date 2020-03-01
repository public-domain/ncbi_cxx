# $Id: Makefile.asniotest.app 76320 2006-01-26 17:45:14Z vakatov $
# Author:  Paul Thiessen

# Build application "asniotest"
#################################

APP = asniotest
SRC = asniotest

LIB = \
	pcsubstance \
	ncbimime \
	cdd \
	scoremat \
	cn3d \
	mmdb \
	id1cli id1 \
	seqset $(SEQ_LIBS) \
	pub \
	medline \
	biblio \
	general \
	xser \
	xconnect \
	xutil \
	xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_CMD =
CHECK_COPY = data asniotest.ini
CHECK_TIMEOUT = 500
