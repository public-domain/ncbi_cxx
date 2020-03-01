# $Id: Makefile.blastdbcheck.app 146846 2008-12-02 19:11:39Z camacho $

APP = blastdbcheck
SRC = blastdbcheck blastdb_aux
REGEX_LIBS = xregexp $(PCRE_LIB)
LIB_ = blastinput $(BLAST_DB_DATA_LOADER_LIBS) $(BLAST_LIBS) $(REGEX_LIBS) \
	$(OBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

CFLAGS   = $(FAST_CFLAGS)
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

CPPFLAGS = $(ORIG_CPPFLAGS)
LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin