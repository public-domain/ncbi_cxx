# $Id: Makefile.ftds_lang.app 101768 2007-04-09 18:39:01Z ucko $

APP = ftds_lang
SRC = ftds_lang

LIB  = ncbi_xdbapi_$(ftds8)$(STATIC) $(FTDS8_LIB) dbapi_driver$(STATIC) $(XCONNEXT) xconnect xncbi
LIBS = $(FTDS8_LIBS) $(ICONV_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(FTDS8_INCLUDE) $(ORIG_CPPFLAGS)

REQUIRES = FreeTDS

CHECK_COPY = ftds_lang.ini
CHECK_CMD =
