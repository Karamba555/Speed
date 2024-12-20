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

ip route flush table 100 2>/dev/null

ip rule del fwmark 100 table 100 2>/dev/null
ip rule add fwmark 100 table 100 2>/dev/null

if [ "$CONFIG_MANUFACTURE_KT" = "y" ]; then
ip route del default dev $interface.$index table 100 2>/dev/null
ip route add default dev $interface.$index table 100 2>/dev/null
else
ip route del default via $gateway dev $interface.$index table 100 2>/dev/null
ip route add default via $gateway dev $interface.$index table 100 2>/dev/null
fi

#add nat for 2nd APN here
lan_ip=`nvram_get 2860 lan_ipaddr`
VoIPRegisterServerIP=`nvram_get 2860 VoIPRegisterDomainToIP`
if [ "$VoIPRegisterServerIP" = "" -o "$VoIPRegisterServerIP" = "0.0.0.0" -o "$VoIPRegisterServerIP" = "0" ]; then
	VoIPRegisterServerIP=`nvram_get 2860 VoIPRegisterServerToIP`
	if [ "$VoIPRegisterServerIP" = "" -o "$VoIPRegisterServerIP" = "0.0.0.0" -o "$VoIPRegisterServerIP" = "0" ]; then
		VoIPRegisterServerIP=`nvram_get 2860 VoIPRegisterServerIP`
	fi
fi
if [ "$VoIPFromLanEnable" = "1" ]; then
VoIPRegisterServerIP=`nvram_get 2860 VoIPRegisterServerIP`
fi


iptables -t nat -F VOIP
iptables -t nat -I VOIP -m mark --mark 100 -j SNAT --to-source $ipaddress 2>/dev/null

if [ "$VoIPRegisterServerIP" = "" -o "$VoIPRegisterServerIP" = "0.0.0.0" -o "$VoIPRegisterServerIP" = "0" ]; then
	echo "VoIPRegisterServerIP "$VoIPRegisterServerIP
else

        if [ "$CONFIG_MANUFACTURE_KT" = "y" ]; then
	###iptables -t nat -D POSTROUTING -s $lan_ip/24 -o lte0 -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -D VOIP -s $lan_ip/24 -o $interface.$index -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -D VOIP -s $lan_ip/24 -o $interface.100 ! -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -I VOIP -s $lan_ip/24 -o $interface.100 ! -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -I VOIP -s $lan_ip/24 -o $interface.$index -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
        else
	###iptables -t nat -D POSTROUTING -s $lan_ip/24 -o lte0 -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -D VOIP -s $lan_ip/24 -o lte0.2 -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -D VOIP -s $lan_ip/24 -o lte0 ! -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -I VOIP -s $lan_ip/24 -o lte0 ! -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
	iptables -t nat -I VOIP -s $lan_ip/24 -o lte0.2 -d $VoIPRegisterServerIP -j MASQUERADE  1>/dev/null 2>&1
        fi
	conntrack-tools.sh &
fi
#end


sh_voip_mangle_rules.sh

