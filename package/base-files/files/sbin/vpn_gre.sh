#!/bin/sh


gre_enable=`nvram_get 2860 gre_enable`
gre_mode_layer=`nvram_get 2860 gre_layer`
gre_src_outer_ipaddr=`nvram_get 2860 src_outer_ipaddr`
gre_dst_outer_ipaddr=`nvram_get 2860 dst_outer_ipaddr`
gre_src_tun_ipaddr=`nvram_get 2860 src_tun_ipaddr`
gre_src_tun_netmask=`nvram_get 2860 src_tun_netmask`
gre_dst_tun_ipaddr=`nvram_get 2860 dst_tun_ipaddr`
gre_dst_tun_netmask=`nvram_get 2860 dst_tun_netmask`
dst_netmask_segment=0
gre_operateType=`nvram_get 2860 gre_operateType`
gre_mode_ser_cli=`nvram_get 2860 gre_mode`
wan_name=`nvram_get 2860 wanConnectionInf`

route del -net $gre_dst_outer_ipaddr netmask 255.255.255.255 gw $gre_src_outer_ipaddr 2>/dev/null;
iptables -D INPUT -s $gre_dst_outer_ipaddr -p icmp -m icmp --icmp-type 8 -j ACCEPT 2>/dev/null
iptables -t nat -F VPN_GRE 2>/dev/null;
iptables -D INPUT -i vpn_gre -j ACCEPT 2>/dev/null;
iptables -D INPUT -p gre -j ACCEPT 2>/dev/null;
ip tunnel set vpn_gre down 2>/dev/null;
ip link set vpn_gre down 2>/dev/null;
ip link del vpn_gre 2>/dev/null;
ip tunnel del vpn_gre 2>/dev/null;
brctl delif br0 vpn_gre 2>/dev/null;

if [ "$gre_enable" = "1" ]; then
	iptables -I INPUT -i vpn_gre -j ACCEPT;
	iptables -I INPUT -p gre -j ACCEPT;
else
	return
fi

:<<!
cal_netmask_segment()
{
	if [ "$2" = "255.255.255.255" ]; then
		netmask_segment_length=32
		if [ "$3" = "src" ]; then
			src_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."$3"."$4}'`
		elif [ "$3" = "dst" ]; then
			dst_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."$3"."$4}'`
		fi
	elif [ "$2" = "255.255.255.0" ]; then
		netmask_segment_length=24
		if [ "$3" = "src" ]; then
			src_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."$3"."0}'`
		elif [ "$3" = "dst" ]; then
			dst_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."$3"."0}'`
		fi
	elif [ "$2" = "255.255.0.0" ]; then
		netmask_segment_length=16
		if [ "$3" = "src" ]; then
			src_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."0"."0}'`
		elif [ "$3" = "dst" ]; then
			dst_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."0"."0}'`
		fi
	elif [ "$2" = "255.0.0.0" ]; then
		netmask_segment_length=8
		if [ "$3" = "src" ]; then
			src_netmask_segment=`echo $1 |awk -F. '{print $1"."0"."0"."0}'`
		elif [ "$3" = "dst" ]; then
			dst_netmask_segment=`echo $1 |awk -F. '{print $1"."0"."0"."0}'`
		fi
	else
		netmask_segment_length=24
		if [ "$3" = "src" ]; then
			src_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."$3"."0}'`
		elif [ "$3" = "dst" ]; then
			dst_netmask_segment=`echo $1 |awk -F. '{print $1"."$2"."$3"."0}'`
		fi
	fi
	
	return $netmask_segment_length
}

!

cal_netmask_segment()
{
	if [ "$2" = "255.255.255.255" ]; then
		netmask_segment_length=32
		dst_netmask_segment=`echo $1 | awk -F. '{print $1"."$2"."$3"."$4}'`
	elif [ "$2" = "255.255.255.0" ]; then
		netmask_segment_length=24
		dst_netmask_segment=`echo $1 | awk -F. '{print $1"."$2"."$3"."0}'`
	elif [ "$2" = "255.255.0.0" ]; then
		netmask_segment_length=16
		dst_netmask_segment=`echo $1 | awk -F. '{print $1"."$2"."0"."0}'`
	elif [ "$2" = "255.0.0.0" ]; then
		netmask_segment_length=8
		dst_netmask_segment=`echo $1 | awk -F. '{print $1"."0"."0"."0}'`
	else
		netmask_segment_length=24
		dst_netmask_segment=`echo $1 | awk -F. '{print $1"."$2"."$3"."0}'`
	fi
	
	return $netmask_segment_length
}

case $gre_mode_layer in
	2)
		ip link add vpn_gre type gretap remote $gre_dst_outer_ipaddr local $gre_src_outer_ipaddr ttl 255 pmtudisc;
		brctl addif br0 vpn_gre;
		ip link set vpn_gre up;
                iptables -D INPUT -s $gre_dst_tun_ipaddr/$network_segment_len -p icmp -m icmp --icmp-type 8 -j ACCEPT 
                iptables -I INPUT -s $gre_dst_tun_ipaddr/$network_segment_len -p icmp -m icmp --icmp-type 8 -j ACCEPT 
                iptables -I INPUT -s $gre_dst_outer_ipaddr -p icmp -m icmp --icmp-type 8 -j ACCEPT
		;;
	3)
		ip tunnel add vpn_gre mode gre remote $gre_dst_outer_ipaddr local $gre_src_outer_ipaddr ttl 255;
		ip link set vpn_gre up mtu 1400 ;
		cal_netmask_segment $gre_src_tun_ipaddr $gre_src_tun_netmask
		network_segment_len=$?;
		ip addr add $gre_src_tun_ipaddr/$network_segment_len peer $gre_dst_tun_ipaddr/$network_segment_len dev vpn_gre;
		iptables -t nat -A VPN_GRE -o vpn_gre -j MASQUERADE;
                iptables -D INPUT -s $gre_dst_tun_ipaddr/$network_segment_len -p icmp -m icmp --icmp-type 8 -j ACCEPT 
                iptables -I INPUT -s $gre_dst_tun_ipaddr/$network_segment_len -p icmp -m icmp --icmp-type 8 -j ACCEPT 
                iptables -I INPUT -s $gre_dst_outer_ipaddr -p icmp -m icmp --icmp-type 8 -j ACCEPT

		if [ "$gre_mode_ser_cli" == "0" ]; then
			iptables -t nat -A VPN_GRE -o $wan_name -s $gre_src_tun_ipaddr/$network_segment_len -j MASQUERADE;
		fi
		if [ "$gre_operateType" == "1" ] && [ "$gre_mode_ser_cli" == "1" ]; then
			route add -net $gre_dst_outer_ipaddr netmask 255.255.255.255 gw $gre_src_outer_ipaddr;
			sleep 2;
			route add default gw $gre_dst_tun_ipaddr dev vpn_gre;
		fi
		;;
	*)
		echo "gre mode is invalid"
		;;
esac
