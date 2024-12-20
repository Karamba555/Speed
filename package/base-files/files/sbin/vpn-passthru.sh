#!/bin/sh

. /sbin/config.sh
. /sbin/global.sh

l2tp_pt=`nvram_get 2860 l2tpPassThru`
ipsec_pt=`nvram_get 2860 ipsecPassThru`
pptp_pt=`nvram_get 2860 pptpPassThru`


# note: they must be removed in order

#if [ "$CONFIG_NF_CONNTRACK_SUPPORT" = "y" ]; then
#	rmmod nf_nat_pptp 1>/dev/null 2>&1
#	rmmod nf_conntrack_pptp 1>/dev/null 2>&1
#	rmmod nf_nat_proto_gre 1>/dev/null 2>&1
#	rmmod nf_conntrack_proto_gre 1>/dev/null 2>&1
#else
#	rmmod ip_nat_pptp 1>/dev/null 2>&1
#	rmmod ip_conntrack_pptp 1>/dev/null 2>&1
#fi
#
#if [ "$pptp_pt" = "1" -o "$l2tp_pt" = "1" -o "$ipsec_pt" = "1" ]; then
#if [ "$CONFIG_NF_CONNTRACK_SUPPORT" = "y" ]; then
#	insmod -q nf_conntrack_proto_gre
#	insmod -q nf_nat_proto_gre

#	if [ "$pptp_pt" = "1" ]; then
#		insmod -q nf_conntrack_pptp
#		insmod -q nf_nat_pptp
#	fi
#else
#	insmod -q ip_conntrack_pptp
#	insmod -q ip_nat_pptp
#fi 

if [ "$pptp_pt" = "1" ]; then
        iptables -D FORWARD -p tcp --dport 1723 -j DROP 2>/dev/null
        iptables -D FORWARD -p 47 -j DROP  2>/dev/null
else
        iptables -D FORWARD -p tcp --dport 1723 -j DROP  2>/dev/null
        iptables -D FORWARD -p 47 -j DROP  2>/dev/null
        iptables -I FORWARD -p tcp --dport 1723 -j DROP  2>/dev/null
        iptables -I FORWARD -p 47 -j DROP  2>/dev/null

fi

if [ "$l2tp_pt" = "1" ]; then
        iptables -D FORWARD -p udp --dport 1701 -j DROP  2>/dev/null
else
        iptables -D FORWARD -p udp --dport 1701 -j DROP  2>/dev/null
        iptables -I FORWARD -p udp --dport 1701 -j DROP  2>/dev/null
fi

if [ "$ipsec_pt" = "1" ]; then
        iptables -D FORWARD -p udp --dport 500 -j DROP  2>/dev/null
        iptables -D FORWARD -p udp --dport 4500 -j DROP  2>/dev/null
        iptables -D FORWARD -p 50 -j DROP  2>/dev/null
        iptables -D FORWARD -p 51 -j DROP  2>/dev/null
else
        iptables -D FORWARD -p udp --dport 500 -j DROP  2>/dev/null
        iptables -D FORWARD -p udp --dport 4500 -j DROP  2>/dev/null
        iptables -D FORWARD -p 50 -j DROP  2>/dev/null
        iptables -D FORWARD -p 51 -j DROP  2>/dev/null
        iptables -I FORWARD -p udp --dport 500 -j DROP  2>/dev/null
        iptables -I FORWARD -p udp --dport 4500 -j DROP  2>/dev/null
        iptables -I FORWARD -p 50 -j DROP  2>/dev/null
        iptables -I FORWARD -p 51 -j DROP  2>/dev/null
fi


