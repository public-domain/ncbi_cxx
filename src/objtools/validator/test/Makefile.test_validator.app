###############################
# $Id: Makefile.test_validator.app 118708 2008-01-31 17:58:33Z ucko $
###############################

APP = test_validator
SRC = test_validator
LIB = xvalidate xobjutil valerr submit $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_CMD  = test_validator.sh
CHECK_COPY = current.prt test_validator.sh

REQUIRES = -Cygwin
