#!/bin/sh

# udhcpc script edited by Tim Riker <Tim@Rikers.org>

. /sbin/config.sh
. /sbin/global.sh

VoIPFromLanEnable=`nvram_get 2860 VoIPFromLanEnable`
VoIPRegisterServerIP=`nvram_get 2860 VoIPRegisterServerIP`
VoIPRegisterServerPort=`nvram_get 2860 VoIPRegisterServerPort`


if [ "$VoIPFromLanEnable" = "1" ]; then
        iptables -t mangle -F
        ip route add $VoIPRegisterServerIP dev eth2.2.102 2>/dev/null
else
iptables -t mangle   -D OUTPUT -p udp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727  -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D OUTPUT -p udp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D POSTROUTING -p udp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D POSTROUTING -p udp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D PREROUTING -p udp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D PREROUTING -p udp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null

iptables -t mangle   -D OUTPUT -p tcp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727  -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D OUTPUT -p tcp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D POSTROUTING -p tcp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D POSTROUTING -p tcp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D PREROUTING -p tcp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -D PREROUTING -p tcp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null


iptables -t mangle   -A OUTPUT -p udp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727  -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A OUTPUT -p udp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A POSTROUTING -p udp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A POSTROUTING -p udp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A PREROUTING -p udp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A PREROUTING -p udp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null

iptables -t mangle   -A OUTPUT -p tcp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727  -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A OUTPUT -p tcp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A POSTROUTING -p tcp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A POSTROUTING -p tcp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A PREROUTING -p tcp -m multiport --sports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
iptables -t mangle   -A PREROUTING -p tcp -m multiport --dports $VoIPRegisterServerPort,4569,10000,20000,2727   -j MARK --set-mark 100 2>/dev/null
fi

ip route flush cache 2>/dev/null 


