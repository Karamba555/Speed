#!/bin/sh

Usage()
{
	echo "Usage: $0 <command>"
	echo "  <command>: "
	echo "		  init - initialize minupnpd"
	echo "		  remove - Remove minupnpd"
	echo "Example:"
	echo "	$0 init"
	echo "	$0 remove"
	exit 1
}

if [ "$1" != "init" -a "$1" != "remove" ]; then
	echo "Unknown command!"
	Usage
	exit 1
fi

. /sbin/global.sh
. /sbin/config.sh

IPTABLES=iptables
WAN_IF=$wan_ppp_if
killall -q miniupnpd 1>/dev/null 2>&1
rm /etc/$MINIUPNPD_FILE 1>/dev/null 2>&1

$IPTABLES -t nat -F MINIUPNPD 1>/dev/null 2>&1
#rmeoving the rule to MINIUPNPD
$IPTABLES -t nat -D PREROUTING -i $WAN_IF -j MINIUPNPD 1>/dev/null 2>&1
$IPTABLES -t nat -X MINIUPNPD 1>/dev/null 2>&1

#removing the MINIUPNPD chain for filter
$IPTABLES -t filter -F MINIUPNPD 1>/dev/null 2>&1
#adding the rule to MINIUPNPD
$IPTABLES -t filter -D FORWARD -i $WAN_IF ! -o $WAN_IF -j MINIUPNPD 1>/dev/null 2>&1
$IPTABLES -t filter -X MINIUPNPD 1>/dev/null 2>&1

IGD=`nvram_get 2860 upnpEnabled`
if [ "$1" == "init" -a "$IGD" == "1" ]; then
	MINIUPNPD_FILE=/etc/miniupnpd.conf
	LAN_IPADDR=`nvram_get 2860 lan_ipaddr`
	WPS1=`nvram_get 2860 WscModeOption`
	WPS2=`nvram_get rtdev WscModeOption`
	upnp_debug=`nvram_get 2860 upnp_debug`
	PORT=0

	#$IPTABLES -N MINIUPNPD
	$IPTABLES -t nat -N MINIUPNPD
	$IPTABLES -t nat -A PREROUTING -i $WAN_IF -j MINIUPNPD
	$IPTABLES -t filter -N MINIUPNPD
	$IPTABLES -t filter -A FORWARD -i $WAN_IF ! -o $WAN_IF -j MINIUPNPD

	echo "ext_ifname=$WAN_IF

listening_ip=$LAN_IPADDR

port=$PORT

enable_natpmp=yes

enable_upnp=yes

bitrate_up=800000000

bitrate_down=800000000

secure_mode=yes

system_uptime=yes

notify_interval=30

uuid=3d3cec3a-8cf0-11e0-98ee-001a6bd2d07b

allow 1-65535 0.0.0.0/0 1-65535
deny 0-65535 0.0.0.0/0 0-65535

serial=12345678
model_number=1
" > $MINIUPNPD_FILE
	#/usr/sbin/miniupnpd -f $MINIUPNPD_FILE -i $WAN_IF  -N   1>/dev/null 2>&1 &
	
	[ "$WPS1" != "" -a "$WPS1" != "0" ] && miniupnpd -m 1 -I ra0 -P /var/run/miniupnpd.ra0 -i $WAN_IF -a $LAN_IPADDR -n 7777 -l $upnp_debug 1>/dev/null 2>&1 &
	[ "$WPS2" != "" -a "$WPS2" != "0" ] && miniupnpd -m 1 -I rai0 -P /var/run/miniupnpd.rai0 -i $WAN_IF -a $LAN_IPADDR -n 8888 -l $upnp_debug 1>/dev/null 2>&1 &
fi
