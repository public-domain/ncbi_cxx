#################################
# $Id: Makefile.netscheduled.app 143081 2008-10-14 22:51:16Z joukovv $
#################################

APP = netscheduled
SRC = netscheduled bdb_queue job_status queue_clean_thread \
      notif_thread ns_affinity squeue access_list ns_util \
      worker_node job

REQUIRES = MT bdb


LIB =  $(BDB_LIB) $(COMPRESS_LIBS) xconnserv xthrserv xconnect xqueryparse \
       xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE) -DBMCOUNTOPT