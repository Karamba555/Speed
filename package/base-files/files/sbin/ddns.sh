#!/bin/sh
#
# $Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/rt2880_app/scripts/ddns.sh#1 $
#
# usage: ddns.sh
#

srv=`nvram_get 2860 DDNSProvider`
ddns=`nvram_get 2860 DDNS`
u=`nvram_get 2860 DDNSAccount`
pw=`subrose_gain 2860 DDNSPassword`
wanInterface=`nvram_get 2860 wanConnectionInf`

killall -q inadyn

if [ "$srv" = "" -o "$srv" = "none" ]; then
	exit 0
fi
if [ "$ddns" = "" -o "$u" = "" -o "$pw" = "" ]; then
	exit 0
fi

if [ -f "/etc/inadyn.conf" ];then
	rm -f /etc/inadyn.conf
	touch /etc/inadyn.conf
fi

# debug
echo "srv=$srv"
echo "ddns=$ddns"
echo "u=$u"
echo "pw=$pw"

echo "iface = "$wanInterface >/etc/inadyn.conf

if [ "$srv" = "dyndns.org" ]; then
	echo "provider dyn {" >>/etc/inadyn.conf
elif [ "$srv" = "freedns.afraid.org" ]; then
	echo "provider freedns {" >>/etc/inadyn.conf
elif [ "$srv" = "zoneedit.com" ]; then
	echo "provider zoneedit.com {" >>/etc/inadyn.conf
elif [ "$srv" = "no-ip.com" ]; then
	echo "provider no-ip.com {" >>/etc/inadyn.conf	
else
	echo "$0: unknown DDNS provider: $srv"
	exit 1
fi

echo "username=\""$u"\"" >>/etc/inadyn.conf
echo "password=\""$pw"\"">>/etc/inadyn.conf
echo "hostname="$ddns>>/etc/inadyn.conf
echo "}" >>/etc/inadyn.conf
inadyn &

#if [ "$srv" = "dyndns.org" ]; then
#	inadyn -u $u -p $pw -a $ddns --dyndns_system dyndns@$srv --update_period_sec 86400 &
#elif [ "$srv" = "freedns.afraid.org" ]; then
#	inadyn -u $u -p $pw -a $ddns --dyndns_system default@$srv --update_period_sec 86400 &
#elif [ "$srv" = "zoneedit.com" ]; then
#	inadyn -u $u -p $pw -a $ddns --dyndns_system default@$srv --update_period_sec 86400 &
#elif [ "$srv" = "no-ip.com" ]; then
#	inadyn -u $u -p $pw -a $ddns --dyndns_system default@$srv --update_period_sec 86400 &
#else
#	echo "$0: unknown DDNS provider: $srv"
#	exit 1
#fi

