#! /bin/sh
# $Id: test_threaded_client_server.sh 48737 2004-10-18 18:15:09Z ucko $

status=0
port="565`expr $$ % 100`"

test_threaded_server -port $port &
server_pid=$!
trap 'kill $server_pid' 1 2 15

sleep 1
$CHECK_EXEC test_threaded_client -port $port -requests 34 || status=1

kill $server_pid # || status=2

exit $status
