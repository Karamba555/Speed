#!/bin/sh
. /sbin/config.sh
. /sbin/config-wifi.sh

#wifi_off_2g=`nvram_get 2860 WiFiOff`
#wifi_off_5g=`nvram_get rtdev WiFiOff`
#guest_wifi_2g_en=`nvram_get 2860 WLAN_SSID2_Enable`
#guest_wifi_5g_en=`nvram_get rtdev WLAN_SSID2_Enable`
#ra_Bssidnum=`nvram_get 2860 BssidNum`
#rai_Bssidnum=`nvram_get rtdev BssidNum`
#ez_enable_2G=`nvram_get 2860 EzEnable`
#mesh_mode_2G=`nvram_get 2860 MeshMode`
#ez_enable_5G=`nvram_get rtdev EzEnable`
#mesh_mode_5G=`nvram_get rtdev MeshMode`
#band_index=`nvram_get 2860 WLAN_BAND_INDEX`
#txpower_2g=`nvram_get 2860 TxPower`

wifi_off_2g=0
wifi_off_5g=0
guest_wifi_2g_en=0
guest_wifi_5g_en=0
ra_Bssidnum=2
rai_Bssidnum=2
ez_enable_2G=""
mesh_mode_2G=""
ez_enable_5G=""
mesh_mode_5G=""
band_index=3
txpower_2g=$(uci get wireless.radio0.txpower 2>/dev/null || true)

# Default region value
region=13

# Function to set the region based on country code
set_region_for_5g() {
    case "$country" in
        JO) region=10 ;;
        TZ|CV|BD|CU|BZ|IR) region=4 ;;
        HK|HM|KP|LC|KN|NC|NA|MY|NF|NE|MZ) region=7 ;;
        CC|CA|CD|AU|RU) region=14 ;;
        EH|VA|TN|EG|KW|TO|AM|GE|MA|UZ|ER|AZ) region=2 ;;
        SM|SK|SE|RO|PT|PL|NO|NL|MT|MD|MC|LV|LT|IT|IS|IE|HU|HR|GR|GB|FR|ES|EE|DK|CZ|CH|BG|BE|AT|DE|MS|WS|SC|ZW|YT|KH|WF|MF|VC|GP|JP|AX|KI|AI|SA|BY|MQ|GQ|ZA|AF|AN|GF|ST|SR|TF|BV|PF|GN|PN|AG|GG|AO|TG|BW|AW|GL|BL|BT|SD|TD|MW|ET|GM|MG|ZM|RE|PM|OM|MR|KM|TR|SB|AQ|DZ) region=1 ;;
        NP|BH|VG|UY|SV|BI|BN|CN|DO|NR|NU|CL|MV|GT|DM|CM|VE|BB|BJ|ID) region=0 ;;
        BO|NG) region=3 ;;
        KG|KE) region=11 ;;
        *) region=13 ;;  # Default region for unknown countries
    esac
}

addRax2Br0()
{	
	if [ "$1" == "ra" ]; then
		nvram_zone="2860"
	elif [ "$1" == "rai" ]; then
		nvram_zone="rtdev"
	fi
	ifconfig "$1""0" 0.0.0.0 1>/dev/null 2>&1
	brctl addif br-lan "$1""0" 1>/dev/null 2>&1
	idx=0
	ssid_idx=0
        echo "Get nvram params"
#	while [ $idx -lt $2 ]; do
		ssid_idx=`expr $idx + 1`
               
                if [ "$ssid_idx" == "1" ]; then
                    ra_enable=1
                    if [ "$nvram_zone" == "2860" ]; then
                        ssid=$(uci get wireless.default_radio0.ssid 2>/dev/null || true)
                        Channel=$(uci get wireless.radio0.channel 2>/dev/null || true)
                        mode=$(uci get wireless.radio0.htmode 2>/dev/null || true)
                        
                        if [[ $mode == *"HE"* ]]; then
                            WirelessMode=16
                        elif [[ $mode == *"HT"* ]]; then
                            WirelessMode=9
                        else
                            WirelessMode=4    
                        fi     
                        
                        encryption=$(uci get wireless.default_radio0.encryption 2>/dev/null || true) 
                        key=$(uci get wireless.default_radio0.key 2>/dev/null || true)
                        radius_server=$(uci get wireless.default_radio0.server 2>/dev/null || true) 
                        radius_port=$(uci get wireless.default_radio0.port 2>/dev/null || true)       
                        radius_key=$(uci get wireless.default_radio0.auth_secret 2>/dev/null || true)
                        ieee80211w=$(uci get wireless.default_radio0.ieee80211w 2>/dev/null || true)
                        #no_scan=$(uci get wireless.radio0.noscan 2>/dev/null || true)
                        cfg_country=$(uci get wireless.default_radio0.country 2>/dev/null || true)
                        country="${cfg_country:-00}"  # Default to '00' if cfg_country is not set
                        region=1
                        if [ "$country" = "TW" ] || [ "$country" = "US" ] || [ "$country" = "CA" ]; then
                            region=0
                        elif [ "$country" = "IL" ]; then
                            region=6
                        fi                       
                        
                    elif [ "$nvram_zone" == "rtdev" ]; then 
                        ssid=$(uci get wireless.default_radio1.ssid 2>/dev/null || true)
                        Channel=$(uci get wireless.radio1.channel 2>/dev/null || true)
                        mode=$(uci get wireless.radio1.htmode 2>/dev/null || true)
                        
                        if [[ $mode == *"HE"* ]]; then
                            WirelessMode=17
                        elif [[ $mode == *"VHT"* ]]; then
                            WirelessMode=14   
                        elif [[ $mode == *"HT"* ]]; then
                            WirelessMode=8
                        else
                            WirelessMode=2
                        fi                        
                        
                        encryption=$(uci get wireless.default_radio1.encryption 2>/dev/null || true)
                        key=$(uci get wireless.default_radio1.key 2>/dev/null || true)
                        radius_server=$(uci get wireless.default_radio1.server 2>/dev/null || true)
                        radius_port=$(uci get wireless.default_radio1.port 2>/dev/null || true)
                        radius_key=$(uci get wireless.default_radio1.auth_secret 2>/dev/null || true)
                        ieee80211w=$(uci get wireless.default_radio1.ieee80211w 2>/dev/null || true)
                        #no_scan=$(uci get wireless.radio1.noscan 2>/dev/null || true)
                        cfg_country=$(uci get wireless.default_radio1.country 2>/dev/null || true)    
                        country="${cfg_country:-00}"  # Default to '00' if cfg_country is not set                   
                        set_region_for_5g
                    fi  
                else
                    ra_enable=0
                fi
                echo "ssid_idx: $ssid_idx"
                echo "ra_enable: $ra_enable"                            

		[ "$ra_enable" == "1" ] && {
		
		        if [ "$encryption" == "psk2" ]; then
		            encryption="WPA2PSK"
		        elif [ "$encryption" == "sae" ]; then
		            encryption="WPA3PSK"
		        elif [ "$encryption" == "sae-mixed" ]; then
		            encryption="WPA2PSKWPA3PSK"
			elif [ "$encryption" == "wpa2+ccmp" ]; then
			    encryption="WPA2"                     
			elif [ "$encryption" == "wpa3" ]; then
			    encryption="WPA3-192"
			elif [ "$encryption" == "wpa3-mixed" ]; then
			    encryption="WPA2WPA3"
		        else
		            encryption="WPA2PSK"
		        fi 
		        
		        if [ "$ieee80211w" == "0" ]; then
		            PMFMFPC='0'
		            PMFMFPR='0'
		        elif [ "$ieee80211w" == "1" ]; then
		            PMFMFPC='1'
		            PMFMFPR='0'    
		        elif [ "$ieee80211w" == "2" ]; then
		            PMFMFPC='1'
		            PMFMFPR='1'
		        else
		            PMFMFPC='0'
		            PMFMFPR='0'                       
		        fi    
                
		        bw="${mode//[!0-9]/}" 
		        if [ "$bw" == "20" ]; then
		           HT_BW=0
		           VHT_BW=0
		        elif [ "$bw" == "40" ]; then
		           HT_BW=1
		           VHT_BW=0
		           #if [ "$no_scan" == "1" ]; then
		           #    HtBssCoex=0
		           #else
		           #    HtBssCoex=1
		           #fi
		        elif [ "$bw" == "80" ]; then  
		           HT_BW=1
		           VHT_BW=1                        
		        elif [ "$bw" == "160" ]; then
		           HT_BW=1
		           VHT_BW=2
			else  # legacy
			   HT_BW=0
			   VHT_BW=0 
		        fi 		
		
		        ifconfig "$1""$idx" 0.0.0.0 1>/dev/null 2>&1
		        brctl addif br-lan "$1""$idx" 1>/dev/null 2>&1
                if [ "$encryption" == "WPA3-192" ]; then
                    iwpriv "$1""$idx" set EncrypType=GCMP256
                else 
                    iwpriv "$1""$idx" set EncrypType=AES
                fi                
                        iwpriv "$1""$idx" set AuthMode=$encryption
                        iwpriv "$1""$idx" set WPAPSK=$key
		        iwpriv "$1""$idx" set SSID=$ssid
		        if [ "$Channel" == "auto" ]; then
		            iwpriv "$1""$idx" set AutoChannelSel=3
		        else
		            iwpriv "$1""$idx" set Channel=$Channel
		        fi    
		        iwpriv "$1""$idx" set WirelessMode=$WirelessMode
		        iwpriv "$1""$idx" set HtBw=$HT_BW
		        iwpriv "$1""$idx" set VhtBw=$VHT_BW
		        iwpriv "$1""$idx" set HT_BW=$HT_BW
		        iwpriv "$1""$idx" set VHT_BW=$VHT_BW
                        iwpriv "$1""$idx" set IEEE8021X=0
			iwpriv "$1""$idx" set RADIUS_Server=$radius_server
			iwpriv "$1""$idx" set RADIUS_Port=$radius_port
			iwpriv "$1""$idx" set RADIUS_Key=$radius_key
			#iwpriv "$1""$idx" set CountryString=UNITED STATES
                        iwpriv "$1""$idx" set PMFMFPC=$PMFMFPC                                                                                          
                        iwpriv "$1""$idx" set PMFMFPR=$PMFMFPR                                                                                          
                        iwpriv "$1""$idx" set PMFSHA256=0
                        #iwpriv "$1""$idx" set HtBssCoex=$HtBssCoex
                        
                        if [ "$nvram_zone" == "2860" ]; then
                            iwpriv "$1""$idx" set CountryRegion=$region
                        elif [ "$nvram_zone" == "rtdev" ]; then 
                            iwpriv "$1""$idx" set CountryRegionABand=$region
                        fi    
                        iwpriv "$1""$idx" set CountryCode=$country
						
                        if [ "$encryption" == "WPA2" -o "$encryption" == "WPA3" -o "$encryption" == "WPA3-192" -o "$encryption" == "WPA2WPA3" ]; then
			    ifconfig "$1""1" 0.0.0.0 1>/dev/null 2>&1
			    iwpriv "$1""$idx" set EAPifname=br-lan
			    iwpriv "$1""$idx" set PreAuthifname=br-lan
                            iwpriv "$1""1" set EAPifname=br-lan
                            iwpriv "$1""1" set PreAuthifname=br-lan
                            check_proc=$(ps | grep "/usr/bin/8021xd -p $1 -i ${1}${idx}" | grep -v "grep")
                            if [ "$check_proc" == "" ]; then 
                                /usr/bin/8021xd -p "$1" -i "$1""$idx" &
                            fi
 			    ifconfig "$1""1" down 1>/dev/null 2>&1 
			else
                            check_proc=$(ps | grep "/usr/bin/8021xd -p $1 -i ${1}${idx}" | grep -v "grep") 
                            if [ "$check_proc" != "" ]; then                         
                                killall /usr/bin/8021xd             
                            fi                         
                        fi
		}

		idx=$ssid_idx
#	done
}


#addrax()
#{
#	apclienable=`nvram_get 2860 ApCli_Enable`
#        #apclienable=""
#	if [ "$apclienable" == "Enable" -a "$CONFIG_RT2860V2_AP_MBSS" == "y" ]; then
#		if [ "$CONFIG_ATEL_IDU_REMOTE_CONTROL" == "y" ]; then
#			killall -9 atelIDURM;atelIDURM & 1>/dev/null 2>&1 
#		fi
#	fi
#
#	if [ "$apclienable" == "Enable" -a "$CONFIG_USER_APCLI_MBSSID" == "y" ]; then
#		killall -9 check_apcli >/dev/null 2>&1
#		check_apcli &
#		ifconfig apcli0 up
#	fi
#}

macaddr_set() 
{
    settings_changed=0
    wifi0MACcorr=$(cat /tmp/nextivity/wifimac | grep default_radio0 | awk -F'default_radio0=' '{print $2}')
    wifi0MAC=$(uci get wireless.default_radio0.macaddr 2>/dev/null || true)
    if [ ! -z "$wifi0MACcorr" -o "$wifi0MACcorr" != "$wifi0MAC" ] ; then
        uci set wireless.default_radio0.macaddr="$wifi0MACcorr"
        settings_changed=1
    fi

    wifi1MACcorr=$(cat /tmp/nextivity/wifimac | grep default_radio1 | awk -F'default_radio1=' '{print $2}')
    wifi1MAC=$(uci get wireless.default_radio1.macaddr 2>/dev/null || true)
    if [ ! -z "$wifi1MACcorr" -o "$wifi1MACcorr" != "$wifi1MAC" ] ; then
        uci set wireless.default_radio1.macaddr="$wifi1MACcorr"
        settings_changed=1
    fi

    if [ $settings_changed -eq 1 ] ; then
        uci commit
        wifi reload
    fi
}


# set wireless mac addresses defined in mtd
macaddr_set

ralink_init make_wireless_config rt2860
[ "$CONFIG_RTDEV" != "" ] && ralink_init make_wireless_config rtdev

# 1: 2.4g only
# 2: 5g only
# 3: 2.4&5g
if [ "$band_index" = "1" ]; then
	[ "$wifi_off_2g" != "1" -o  "$guest_wifi_2g_en" == "1" ] && addRax2Br0 ra $ra_Bssidnum
elif [ "$band_index" = "2" ]; then
	if [ "$CONFIG_RTDEV" != "" ]; then
		[ "$wifi_off_5g" != "1"  -o  "$guest_wifi_5g_en" == "1" ] && addRax2Br0 rai $rai_Bssidnum
	fi
else
    [ "$wifi_off_2g" != "1" -o  "$guest_wifi_2g_en" == "1"  ] && addRax2Br0 ra $ra_Bssidnum
    if [ "$CONFIG_RTDEV" != ""  ]; then
        [ "$wifi_off_5g" != "1"  -o  "$guest_wifi_5g_en" == "1"  ] && addRax2Br0 rai $rai_Bssidnum
    fi
fi

echo "Set  TX Power"
iwpriv ra0 set TxPower=$txpower_2g

#if [ "$CONFIG_RTDEV" != "" ]; then
#	#[ "$ez_enable_2G" == "1" -a "$mesh_mode_2G" == "0" ] && ifconfig apcli0 up
#	#[ "$ez_enable_5G" == "1" -a "$mesh_mode_5G" == "0" ] && ifconfig apclii0 up
#	[ "$ez_enable_2G" == "1" -a "$mesh_mode_2G" == "1" ] && iwpriv ra0 set ez_connection_allow_all=
#	[ "$ez_enable_5G" == "1" -a "$mesh_mode_5G" == "1" ] && iwpriv rai0 set ez_connection_allow_all=
#fi


#if [ "$CONFIG_USER_VLAN" == "y" ]; then
#	#wifiDhcp2G=`nvram_get 2860 wifiDhcp2G`
#	#wifiDhcp5G=`nvram_get 2860 wifiDhcp5G`
#       wifiDhcp2G=""
#        wifiDhcp5G=""
#	if [ "$wifiDhcp2G" != "" -o "$wifiDhcp5G" != "" ]; then
#		#debug
#		sh -x /sbin/config-wifi-dhcp.sh init  2>/tmp/config-wifi-dhcp-debugwifi.log 1>>/tmp/config-wifi-dhcp-debugwifi.log&
#		#config-wifi-dhcp.sh init 2>/dev/null 1>/dev/null
#		config-wifi-isolate.sh 0
#		config-wifi-isolate.sh 1
#	fi
#fi

a=0
while true; do
    a=1
done

# iwpriv ra0 set Channel=$Channel
# iwpriv rai0 set Channel=$Channel
