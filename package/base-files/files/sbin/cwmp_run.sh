#!/bin/sh
srv=`nvram_get 2860 tr069En`
killall -q user
killall -q msgd
killall -q cwmp
if [ "$srv" = "1" ]; then
	sleep 2
	echo "runing msgd"
	/bin/msgd &
	sleep 2
	echo "runing user"
	/bin/user &
else
	exit 1
fi

