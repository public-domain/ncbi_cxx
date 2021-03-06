#! /bin/sh
#
# $Id: test_conn_stream_pushback.sh 145912 2008-11-18 16:44:58Z lavr $
#

exit_code=0
log=test_conn_stream_pushback.log
rm -f $log

trap 'rm -f $log; echo "`date`."' 0 1 2 15

i=0
j="`expr $$ % 2 + 1`"
while [ $i -lt $j ]; do
  echo "${i} of ${j}: `date`"
  $CHECK_EXEC test_conn_stream_pushback >$log 2>&1
  exit_code=$?
  if [ "$exit_code" != "0" ]; then
      head -20 $log
      echo '......'
      tail -20 $log
      break
  fi
  i="`expr $i + 1`"
done
exit $exit_code
