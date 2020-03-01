# $Id: Makefile.test_ncbi_os_unix.app 78651 2006-03-10 19:15:53Z ivanov $

APP = test_ncbi_os_unix
SRC = test_ncbi_os_unix
LIB = xncbi

REQUIRES = unix -Cygwin

CHECK_CMD  = test_ncbi_os_unix.sh
CHECK_COPY = test_ncbi_os_unix.sh

