# $Id: Makefile.test_cgi_entry_reader.app 138116 2008-08-21 19:06:18Z ivanovp $

APP = test_cgi_entry_reader
SRC = test_cgi_entry_reader
LIB = xcgi xutil xncbi

CHECK_CMD = test_cgi_entry_reader.sh test_cgi_entry_reader.dat /CHECK_NAME=test_cgi_entry_reader.sh
CHECK_COPY = test_cgi_entry_reader.sh test_cgi_entry_reader.dat
