#!/bin/sh
#
# usage: vlan-iptables.sh
#
#filter table
iptables -t filter -N vlan_filter
iptables -t filter -N vlan_forward
iptables -t filter -A INPUT -j vlan_filter
iptables -t filter -A FORWARD -j vlan_forward
iptables -F vlan_filter
iptables -F vlan_forward
#nat table
iptables -t nat -D POSTROUTING -j vlan_nat
iptables -t nat -F vlan_nat  
iptables -t nat -X vlan_nat 
iptables -t nat -N vlan_nat
iptables -t nat -A POSTROUTING -j vlan_nat             

vids=`nvram_get 2860 vlan_vids`
for vid in $(echo $vids | tr ";" "\n"); do
if [ "$vid" != "1" ]; then
iptables -A vlan_filter -i "eth2."$vid -j ACCEPT
#iptables -t nat -A vlan_nat -s "192.168."$vid".0/24" -o $wanInterface -j MASQUERADE
fi
done
wanInterface=`nvram_get 2860 wanConnectionInf`
vlan_dhcps=`nvram_get 2860 vlan_dhcps`
if [ "$vlan_dhcps" != "" ];then
	for vlan_dhcp in $(echo $vlan_dhcps | tr ";" "\n")
	do
		vlan_gateway=`echo $vlan_dhcp | awk -F"," '{print $3}'`
		vlan_net=`echo ${vlan_gateway%.*}`
		iptables -t nat -A vlan_nat -s $vlan_net".0/24" -o $wanInterface -j MASQUERADE  #TODO x.x.x.0/24 need compute  
	done
fi
isolate_vlans=`nvram_get 2860 isolate_vlans`
for isolate_vlan_tmp in $(echo $isolate_vlans | tr ";" "\n"); do
	
vid_r=`echo $isolate_vlan_tmp | awk -F"," '{print $1}'`
vid_c=`echo $isolate_vlan_tmp | awk -F"," '{print $2}'`
if [ "$vid_c" == "1" -o "$vid_r" == "1" ]; then
	if [ "$vid_c" == "1" ]; then
		iptables -I vlan_forward -i eth2.$vid_r -o br0 -j DROP
	elif [ "$vid_r" == "1" ]; then
		iptables -I vlan_forward -i br0 -o eth2.$vid_c -j DROP
	fi
else
	iptables -I vlan_forward -i eth2.$vid_r -o eth2.$vid_c -j DROP
fi
done
