#!/bin/sh
#
#

tz=`nvram_get 2860 TZ`

##hwclock -u -w
date "+%Y-%m-%d %H:%M:%S" >/tmp/currentTime
ntpvalid4=`cat /tmp/currentTime`
echo "export TZ=$tz"  >/etc/profile
export TZ=$tz
date -s  "$ntpvalid4"
hwclock -s


