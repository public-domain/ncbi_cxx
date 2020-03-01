#################################
# $Id: Makefile.test_ncbimime.app 24813 2003-07-17 20:04:06Z vasilche $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build test application "test_ncbimime"
#################################

APP = test_ncbimime
SRC = test_ncbimime
LIB = ncbimime cdd scoremat cn3d mmdb seqset $(SEQ_LIBS) \
	pub medline medlars biblio pub general \
	xser xutil xncbi
