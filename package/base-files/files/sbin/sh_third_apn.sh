#!/bin/sh

# udhcpc script edited by Tim Riker <Tim@Rikers.org>

. /sbin/config.sh
. /sbin/global.sh
[ -z "$1" ] && echo "Error: sh_second_apn interface index ipaddress,netmask,gateway,dns1,dns2" && exit 1

interface=$1
index=$2
ipaddress=$3
netmask=$4
gateway=$5
dns1=$6
dns2=$7

vconfig add $interface $index 2>/dev/null

ifconfig $interface.$index $ipaddress netmask $netmask up 2>/dev/null

conntrack-tools.sh &
killall -9 easycwmpd
init_system restart_easycwmpd &

