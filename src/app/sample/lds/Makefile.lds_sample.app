#################################
# $Id: Makefile.lds_sample.app 151845 2009-02-09 14:50:16Z ludwigf $
#################################

REQUIRES = objects bdb

APP = lds_sample
SRC = lds_sample

# new_project.sh will copy everything in the following block to any
# Makefile.*_app generated from this sample project.  Do not change
# the lines reading "### BEGIN/END COPIED SETTINGS" in any way.

### BEGIN COPIED SETTINGS
LIB = ncbi_xloader_lds lds xobjread bdb xobjutil $(SOBJMGR_LIBS)

LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)
### END COPIED SETTINGS
