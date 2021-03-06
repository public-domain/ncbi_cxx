# $Id: third_party_static_install.mak 154603 2009-03-12 18:57:16Z ivanov $
#################################################################


INSTALL          = .\bin
INSTALL_BINPATH  = $(INSTALL)\$(INTDIR)
THIRDPARTY_MAKEFILES_DIR =  .


META_MAKE = $(THIRDPARTY_MAKEFILES_DIR)\..\third_party_install.meta.mk
!IF EXIST($(META_MAKE))
!INCLUDE $(META_MAKE)
!ELSE
!ERROR  $(META_MAKE)  not found
!ENDIF

THIRD_PARTY_LIBS = \
	install_berkeleydb \
	install_fltk       \
	install_gnutls     \
	install_lzo        \
	install_mssql      \
	install_mysql      \
	install_openssl    \
	install_sqlite     \
	install_sqlite3    \
	install_sybase     \
	install_wxwidgets  \
	install_wxwindows  \
	install_xalan      \
	install_xerces     

CLEAN_THIRD_PARTY_LIBS = \
	clean_berkeleydb \
	clean_fltk       \
	clean_gnutls     \
	clean_lzo        \
	clean_mssql      \
	clean_mysql      \
	clean_openssl    \
	clean_sqlite     \
	clean_sqlite3    \
	clean_sybase     \
	clean_wxwidgets  \
	clean_wxwindows  \
	clean_xalan      \
	clean_xerces

all : dirs $(THIRD_PARTY_LIBS)

clean : $(CLEAN_THIRD_PARTY_LIBS)

rebuild : clean all

dirs :
    @if not exist $(INSTALL_BINPATH) (echo Creating directory $(INSTALL_BINPATH)... & mkdir $(INSTALL_BINPATH))
