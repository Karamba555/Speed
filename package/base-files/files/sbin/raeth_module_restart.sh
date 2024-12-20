#!/bin/sh
#
# $Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/rt2880_app/scripts/internet.sh#7 $
#
# usage: internet.sh
#

. /sbin/config.sh
. /sbin/global.sh

set_vlan_map()
{
	# vlan priority tag => skb->priority mapping
	vconfig set_ingress_map $1 0 0
	vconfig set_ingress_map $1 1 1
	vconfig set_ingress_map $1 2 2
	vconfig set_ingress_map $1 3 3
	vconfig set_ingress_map $1 4 4
	vconfig set_ingress_map $1 5 5
	vconfig set_ingress_map $1 6 6
	vconfig set_ingress_map $1 7 7

	# skb->priority => vlan priority tag mapping
	vconfig set_egress_map $1 0 0
	vconfig set_egress_map $1 1 1
	vconfig set_egress_map $1 2 2
	vconfig set_egress_map $1 3 3
	vconfig set_egress_map $1 4 4
	vconfig set_egress_map $1 5 5
	vconfig set_egress_map $1 6 6
	vconfig set_egress_map $1 7 7
}

configVIF()
{
	echo "##### configVIF #####"

	ifconfig eth2 0.0.0.0
	vconfig add eth2 1
	set_vlan_map eth2.1
	vconfig add eth2 2
	set_vlan_map eth2.2
	
	ifconfig eth2.2 down
	wan_mac=`nvram_get 2860 WanAddress`
	if [ "$wan_mac" != "FF:FF:FF:FF:FF:FF" ]; then
		ifconfig eth2.2 hw ether $wan_mac
	fi

	ifconfig eth2.1 0.0.0.0
	ifconfig eth2.2 0.0.0.0
	if [ "$CONFIG_RAETH_SPECIAL_TAG" == "y" ]; then
		ifconfig eth2.3 0.0.0.0
		ifconfig eth2.4 0.0.0.0
		ifconfig eth2.5 0.0.0.0
	fi
}

ifconfig eth2 down

if [ "$CONFIG_RAETH" = "m" ]; then
rmmod raeth
insmod -q raeth
fi

configVIF

if [ "$CONFIG_RAETH_ROUTER" = "y" -a "$CONFIG_LAN_WAN_SUPPORT" = "y" ]; then
	if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
		echo '##### config IC+ vlan partition (WLLLL) #####'
		config-vlan.sh 0 WLLLL 1>/dev/null 2>&1 
	else
		echo '##### config IC+ vlan partition (LLLLW) #####'
		config-vlan.sh 0 LLLLW 1>/dev/null 2>&1 
	fi
fi
if [ "$CONFIG_MAC_TO_MAC_MODE" = "y" ]; then
	if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
		echo '##### config Vtss vlan partition (WLLLL) #####'
		config-vlan.sh 1 WLLLL 1>/dev/null 2>&1 
	else
		echo '##### config Vtss vlan partition (LLLLW) #####'
		config-vlan.sh 1 LLLLW 1>/dev/null 2>&1 
	fi
	if [ "$CONFIG_IPV6" == "y" -o "$CONFIG_IPV6" == "m" ]; then
		sleep 3
	fi
fi
if [ "$CONFIG_RT_3052_ESW" = "y" -a "$CONFIG_LAN_WAN_SUPPORT" = "y" ]; then
	if [ "$CONFIG_P5_RGMII_TO_MAC_MODE" = "y" -o  "$CONFIG_GE2_RGMII_AN" = "y" -o "$CONFIG_GE2_INTERNAL_GPHY" = "y" ]; then
		echo "##### restore Ralink ESW to dump switch #####"
		if [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" -o "$CONFIG_RALINK_MT7620" = "y" -o "$CONFIG_RALINK_MT7621" = "y" ]; then
			config-vlan.sh 3 0  1>/dev/null 2>&1 
		else
			config-vlan.sh 2 0 1>/dev/null 2>&1 
		fi
		if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
			if [ "$CONFIG_RALINK_MT7621" = "y" ]; then
				if [ "$CONFIG_RAETH_8023AZ_EEE" = "y" ]; then
				echo '##### config Switch vlan partition (WLLLL) #####'
				switch vlan  set 1 1 01111011
				switch vlan  set 2 2 10000100
				switch pvid 0 2
				switch pvid 5 2
				switch reg w 2004 ff0003
				switch reg w 2104 ff0003
				switch reg w 2204 ff0003
				switch reg w 2304 ff0003
				switch reg w 2404 ff0003
				switch reg w 2504 ff0003
				switch reg w 2604 ff0003
				fi
			elif [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" = "y" -o "$CONFIG_RALINK_MT7620" = "y" ]; then
				echo '##### config External Switch vlan partition (WLLLL) #####'
			else
				echo '##### config External Switch vlan partition (WLLLL) #####'
				echo "initialize external switch (WLLLL)"
				config-vlan.sh 1 WLLLL 1>/dev/null 2>&1 
			fi
		else
			if [ "$CONFIG_RALINK_MT7621" = "y" ]; then
				if [ "$CONFIG_RAETH_8023AZ_EEE" = "y" ]; then
				echo '##### config Switch vlan partition (LLLLW) #####'
				switch vlan  set 1 1 11110011
				switch vlan  set 2 2 00001100
				switch pvid 4 2
				switch pvid 5 2
				switch reg w 2004 ff0003
				switch reg w 2104 ff0003
				switch reg w 2204 ff0003
				switch reg w 2304 ff0003
				switch reg w 2404 ff0003
				switch reg w 2504 ff0003
				switch reg w 2604 ff0003
				fi
			elif [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" = "y" -o "$CONFIG_RALINK_MT7620" = "y" ]; then
				echo '##### config External Switch vlan partition (LLLLW) #####'
			else
				echo '##### config External Switch vlan partition (LLLLW) #####'
				echo "initialize external switch (LLLLW)"
				config-vlan.sh 1 LLLLW 1>/dev/null 2>&1 
			fi
		fi
	else
		if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
			echo '##### config Ralink ESW vlan partition (WLLLL) #####'
			if [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" = "y" -o "$CONFIG_RALINK_MT7620" = "y" -o "$CONFIG_RALINK_MT7621" = "y" ]; then
				config-vlan.sh 3 WLLLL 1>/dev/null 2>&1 
			else
				config-vlan.sh 2 WLLLL 1>/dev/null 2>&1 
			fi
		else
			echo '##### config Ralink ESW vlan partition (LLLLW) #####'
			if [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" = "y" -o "$CONFIG_RALINK_MT7620" = "y" -o "$CONFIG_RALINK_MT7621" = "y" ]; then
				config-vlan.sh 3 LLLLW 1>/dev/null 2>&1 
			else
				config-vlan.sh 2 LLLLW 1>/dev/null 2>&1 
			fi
		fi
	fi
fi

if [ "$CONFIG_RAETH_ROUTER" = "y" -o "$CONFIG_MAC_TO_MAC_MODE" = "y" -o "$CONFIG_RT_3052_ESW" = "y" ]; then
	if [ "$CONFIG_RAETH_SPECIAL_TAG" == "y" ]; then
		if [ "$CONFIG_WAN_AT_P4" = "y" ]; then
			brctl addif br0 eth2.1
		fi
		brctl addif br0 eth2.2
		brctl addif br0 eth2.3
		brctl addif br0 eth2.4
		if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
			brctl addif br0 eth2.5
		fi
	elif [ "$CONFIG_GE2_RGMII_AN" = "y" -o "$CONFIG_GE2_INTERNAL_GPHY" = "y" ]; then
		brctl addif br0 eth2
	else
		brctl addif br0 eth2.1
	fi
fi

# IC+ 100 PHY (one port only)
if [ "$CONFIG_ICPLUS_PHY" = "y" ]; then
	echo '##### connected to one port 100 PHY #####'
	#
	# setup ip alias for user to access web page.
	#
	ifconfig eth2:1 172.32.1.254 netmask 255.255.255.0 up
fi
if [ "$CONFIG_GE1_RGMII_AN" = "y" -a "$CONFIG_GE2_RGMII_AN" = "y" ]; then
	echo "##### connected to two Giga PHY port #####"
	brctl addif br0 eth2
fi
