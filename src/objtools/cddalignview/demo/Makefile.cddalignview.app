# $Id: Makefile.cddalignview.app 32976 2004-01-15 22:01:30Z thiessen $
# Author:  Paul Thiessen

# Build application "cddalignview"
#################################

APP = cddalignview

SRC = \
	cav_main

LIB = \
	xcddalignview \
	ncbimime \
	cdd \
	scoremat \
	cn3d \
	mmdb \
	seqset $(SEQ_LIBS) \
	pub \
	medline \
	biblio \
	general \
	xser \
	xutil \
	xncbi
