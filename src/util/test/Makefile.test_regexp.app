#################################
# $Id: Makefile.test_regexp.app 138116 2008-08-21 19:06:18Z ivanovp $
# Author:  Clifford Clausen (clausen@ncbi.nlm.nih.gov)
#################################

# Build test application "test_regex"
#################################
APP = test_regexp
SRC = test_regexp
LIB = xregexp $(PCRE_LIB) xutil xncbi
LIBS = $(PCRE_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(PCRE_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_CMD = test_regexp Abc Ef Ghh Ooo Pppk /CHECK_NAME=test_regexp