# $Id: Makefile.hello.app 100542 2007-03-19 22:17:49Z vakatov $
# Author:  Lewis Geer

# Build CGI application "HELLO"
# NOTE:  see in "Makefile.fasthello.app" for how to build it as Fast-CGI
#################################

APP = hello
SRC = helloapp hellores hellocmd
LIB = xhtml xcgi xutil xncbi

CHECK_CMD =
