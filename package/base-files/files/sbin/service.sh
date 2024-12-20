#!/bin/sh


#don't remove this , yyq
mount -o atelremount,ro / 


UPNP=`nvram_get 2860 upnpEnabled`

if [ "$UPNP" = "1" ];then
    /sbin/miniupnpd.sh init  1>/dev/null 2>&1 &
fi
