# $Id: Makefile.test_jsmenu.app 9305 2001-12-13 18:50:56Z ucko $

# Simple test for the "XHTML", JavaScript popup menus
#################################

APP = test_jsmenu
SRC = test_jsmenu
LIB = xhtml xncbi

CHECK_CMD  = test_jsmenu html
CHECK_CMD  = test_jsmenu template
CHECK_COPY = template_jsmenu.html
