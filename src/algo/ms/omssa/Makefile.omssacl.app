# $Id: Makefile.omssacl.app 142820 2008-10-09 20:46:28Z lewisg $
# Author:  Lewis Y. Geer

# Build application "omssacl"
#################################

CXXFLAGS = $(FAST_CXXFLAGS) $(CMPRS_INCLUDE) $(STATIC_CXXFLAGS)

LDFLAGS  = $(FAST_LDFLAGS) $(STATIC_LDFLAGS) $(RUNPATH_ORIGIN)

APP = omssacl

SRC = omssacl

LIB = xomssa omssa pepXML blast composition_adjustment tables seqdb blastdb \
      xregexp $(PCRE_LIB) xconnect $(COMPRESS_LIBS) $(SOBJMGR_LIBS) 

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)
