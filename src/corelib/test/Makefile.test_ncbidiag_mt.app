# $Id: Makefile.test_ncbidiag_mt.app 115519 2007-12-13 17:17:59Z grichenk $

APP = test_ncbidiag_mt
SRC = test_ncbidiag_mt
LIB = xncbi test_mt

CHECK_CMD = test_ncbidiag_mt -format old
CHECK_CMD = test_ncbidiag_mt -format new
