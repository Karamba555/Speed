#!/bin/sh
#
# $Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/rt2880_app/scripts/ntp.sh#1 $
#
# usage: ntp.sh
#

srv=`nvram_get 2860 NTPServerIP`
sync=`nvram_get 2860 NTPSync`
tz=`nvram_get 2860 TZ`

success=$1


killall -q ntpclient 2>/dev/null
killall -9 ntpclient 2>/dev/null
if [ "$srv" = "" ]; then
	exit 0
fi


#if [ "$sync" = "" ]; then
#	sync=1
#elif [ $sync -lt 300 -o $sync -le 0 ]; then
#	sync=1
#fi

sync=`expr $sync \* 3600`

if [ "$tz" = "" ]; then
	tz="UCT_000"
fi

#debug
#echo "serv=$srv"
#echo "sync=$sync"
#echo "tz=$tz"

echo $tz > /etc/tmpTZ
sed -e 's#.*_\(-*\)0*\(.*\)#GMT-\1\2#' /etc/tmpTZ > /etc/tmpTZ2
sed -e 's#\(.*\)--\(.*\)#\1\2#' /etc/tmpTZ2 > /etc/TZ
#rm -rf /etc/tmpTZ
rm -rf /etc/tmpTZ2

switch_timezone.sh

killall -9 ntpclient 2>/dev/null

if [ "$success" = "success" ]; then
        ntpclient -s -c 0 -h $srv -i $sync -l -h time.windows.com -h pool.ntp.org &
        echo "1" > /tmp/NTPValid
else
        # please don't kill it,it will exit automatic
        ntpclient -s -c 5 -i 10 -l -h $srv -h time.windows.com &
fi
