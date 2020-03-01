# $Id: Makefile.python_ncbi_dbapi_test.app 144246 2008-10-28 18:59:17Z vakatov $

APP = python_ncbi_dbapi_test
SRC = python_ncbi_dbapi_test

REQUIRES = PYTHON Boost.Test

CPPFLAGS = $(ORIG_CPPFLAGS) $(PYTHON_INCLUDE) $(BOOST_INCLUDE)

LIB  = dbapi_driver$(STATIC) xutil xncbi
LIBS = $(BOOST_TEST_LIBS) $(PYTHON_LIBS) $(ORIG_LIBS)

# CHECK_REQUIRES = mswin
CHECK_REQUIRES = DLL
CHECK_CMD = python_ncbi_dbapi_test.sh
CHECK_COPY = python_ncbi_dbapi_test.sh
CHECK_TIMEOUT = 300