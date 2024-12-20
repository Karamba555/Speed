#!/bin/sh

modemstatus=/tmp/nextivity/modemstatus.json
gpsstatus=/tmp/nextivity/gpsstatus.json
wanstatus=/tmp/wan_status

data="120:LTE_BAND_1 121:LTE_BAND_2 122:LTE_BAND_3 123:LTE_BAND_4 124:LTE_BAND_5 125:LTE_BAND_6 126:LTE_BAND_7 
127:LTE_BAND_8 128:LTE_BAND_9 129:LTE_BAND_10 130:LTE_BAND_11 131:LTE_BAND_12 132:LTE_BAND_13 133:LTE_BAND_14 
134:LTE_BAND_17 135:LTE_BAND_33 136:LTE_BAND_34 137:LTE_BAND_35 138:LTE_BAND_36 139:LTE_BAND_37 140:LTE_BAND_38 
141:LTE_BAND_39 142:LTE_BAND_40 143:LTE_BAND_18 144:LTE_BAND_19 145:LTE_BAND_20 146:LTE_BAND_21 147:LTE_BAND_24 
148:LTE_BAND_25 149:LTE_BAND_41 150:LTE_BAND_42 151:LTE_BAND_43 152:LTE_BAND_23 153:LTE_BAND_26 154:LTE_BAND_32 
155:LTE_BAND_125 156:LTE_BAND_126 157:LTE_BAND_127 158:LTE_BAND_28 159:LTE_BAND_29 160:LTE_BAND_30 161:LTE_BAND_66 
162:LTE_BAND_250 163:LTE_BAND_46 166:LTE_BAND_71 167:LTE_BAND_47 168:LTE_BAND_48 250:NR5G_BAND_1 251:NR5"

# $1 - parameter name, $2 - parameter value
update_json() {
  local field="$1" 
  local value="$2"
  
  #field=$(echo $field | tr -d '\r')

  temp_file="/tmp/nextivity/updated_modemstatus.json"

  current_time=$(date +%s)

  # Sanitize the input JSON to escape control characters
  #sed 's/[\x00-\x1F\x7F]/ /g' "$modemstatus" > "$temp_file"

  # Process the file and update the value and lastUpdate time
  awk -v name="$field" -v value="$value" -v time="$current_time" '
  BEGIN { found=0 }
  {
    # Check if the current line contains the target name
    if ($0 ~ "\"name\": \"" name "\"") {
        found=1
    }

    # If the target name was found, modify the value
    if (found && $0 ~ "\"value\":") {
        sub(/\"value\": \"[^\"]*\"/, "\"value\": \"" value "\"")
    }

    # After modifying the value, update lastUpdate
    if (found && $0 ~ "\"lastUpdate\":") {
        sub(/\"lastUpdate\": \"[^\"]*\"/, "\"lastUpdate\": \"" time "\"")
        found=0
    }

    print
  }
  ' "$modemstatus" > "$temp_file" && mv "$temp_file" "$modemstatus"
}

get_gps_value() {
  res=$(grep -A 1 $1 $gpsstatus | tail -n 1 | awk {'print $2'} | awk -F '"' {'print $2'})
  echo $res
}

is_number() {
    case $1 in
        ''|*[!0-9]*)  echo 0 ;;
        *) echo 1 ;;
    esac
}

str_to_hex() {
  string=$1
  hex=""
  i=0
  while [ $i -lt "${#string}" ]; do
      hex="$hex$(printf "%x" "'${string:i:1}")"
      i=$((i + 1))
  done
  echo "$hex"
}

#Control the HPUE LED.  The color depends on the txPwr.
set_hpue_led() {
  txPwr=$1
  tx_power_led_state="green"
  if [ -f /tmp/nextivity/tx_power_led_state ]; then
      tx_power_led_state=$(cat /tmp/nextivity/tx_power_led_state)
  fi

  if [ "$txPwr" -le "23" ]; then
    # Color is green.
    if [ "$tx_power_led_state" == "blue" -o "$tx_power_led_state" == "" ]; then
        gpio led hpue_blue off
        gpio led hpue_green on
        echo "green" > /tmp/nextivity/tx_power_led_state
    fi
  else
    # Color is blue.
    if [ "$tx_power_led_state" == "green" -o "$tx_power_led_state" == "" ]; then
        gpio led hpue_green off
        gpio led hpue_blue on
        echo "blue" > /tmp/nextivity/tx_power_led_state
    fi
  fi
}

update_connection_status() {
    current_state=$1

    is_state_changed=0
    if [ ! -f /tmp/nextivity/conn_led_state ]; then
        is_state_changed=1
    else
        state=$(cat /tmp/nextivity/conn_led_state)
        if [ "$state" != "$current_state" ]; then
            is_state_changed=1
        fi
    fi

    if [ "$is_state_changed" == "1" ]; then
        echo $current_state > /tmp/nextivity/conn_led_state
        connection_status=$(cat $wanstatus | awk -F':' '/Connection Status:/ {print $2}' | awk -F ';' '{print $1}')
        # if 0 : "NULL" or 1 : "DEREGISTERD" or 7 : "INVALID"
        if [ "$current_state" == "0" -o "$current_state" == "1" -o "$current_state" == "7" ]; then
            update_json "Connection_s" "Disconnected"
            sed -i "s/Connection Status:$connection_status;/Connection Status:Disconnected;/g" $wanstatus
            #gpio led red on    
        # if 3 : "REGISTRED"
        elif [ "$current_state" == "3" ]; then
            update_json "Connection_s" "Connected"
	    sed -i "s/Connection Status:$connection_status;/Connection Status:Connected;/g" $wanstatus
            #gpio led green on    
        else 
            update_json "Connection_s" "Connecting"
            sed -i "s/Connection Status:$connection_status;/Connection Status:Connecting;/g" $wanstatus
            #gpio led green blink
        fi
    fi
}

get_status_to_json() {
    wait_ms=10
    baudrate=115200

    # update time
    current_time=$(date +%s)
    jq --argjson time $current_time '.time = $time' $modemstatus > tmp.json && mv tmp.json $modemstatus

    # status time
    update_json "StatusTime_i" $current_time
echo "----------------"
    # get System serial Number
    if [ "$serial_number" = "" ]; then
        if [ -f /tmp/nextivity/serialNumber ]; then
            serial_number=$(cat /tmp/nextivity/serialNumber)
            update_json "Sn_s" $serial_number
        fi
    fi

    # get system name
    if [ "$system_name" = "" ]; then
        if [ -f /etc/nextivity_build_info ]; then
            ES_SYSTEMNAME=$(cat /etc/nextivity_build_info | grep "Target board" | awk {'print $3'})
            ES_SYSTEMNAME=$ES_SYSTEMNAME" MegaFi 2"
            update_json "SystemName_s" $ES_SYSTEMNAME
            system_name=$ES_SYSTEMNAME
        fi
    fi

    # get System SW version
    if [ "$sw_version" = "" ]; then
        if [ -f /etc/nextivity_build_info ]; then
            ES_PCSWV=$(cat /etc/nextivity_build_info | grep "Firmware Version" | awk -F : {'print $2'})
            #echo $ES_PCSWV
            update_json "SwVersion_s" "$ES_PCSWV"
            sw_version=$ES_PCSWV
        fi
    fi

    # get LAN IP Address
    ES_LANIP=$(uci get network.lan.ipaddr)
    update_json "LanIpAddr_s" $ES_LANIP

    # get LAN MAC Address
    ES_LANMAC=$(ifconfig lan1 | grep HWaddr | awk {'print $5'})
    update_json "LanMacAddr_s" $ES_LANMAC

    # get WWAN IP Address
    mode=$(uci get awc.main.bridge_mode 2>/dev/null || true)
    # if passthrough mode
    if [ "$mode" == "on" ]; then
        ES_PDPIP=$(cat /sys/class/net/wwan0/qmi/bridge_ipv4 2>/dev/null || true)
    # nat mode
    else
        ES_PDPIP=$(ifconfig wwan0 | grep "inet addr" | awk {'print $2'} | awk -F : {'print $2'} 2>/dev/null || true)
    fi
    update_json "PdpIpAddr_s" $ES_PDPIP

    # get WWAN MAC Address
    ES_MMACADDR=$(ifconfig wwan0 | grep HWaddr | awk {'print $5'} 2>/dev/null || true)
    update_json "ModemMacAddr_s" $ES_MMACADDR

    # get APN
    ES_APN=$(uci get network.wwan.apn)
    update_json "Apn_s" $ES_APN

    # get modem protocol - qmi
    proto=$(ls /dev | grep "cdc-wdm")
    if [ "$proto" != "" ]; then
        echo "qmi" > /tmp/nextivity/protocol
    fi
    
    if [ -f /tmp/nextivity/protocol ]; then
      ES_PROTOCOL=$(cat /tmp/nextivity/protocol)
    else
      ES_PROTOCOL="unknown"
    fi
    update_json "Protocol_s" $ES_PROTOCOL

    # GetEiNetworkInfo
    ES_EIWWAN0RXBYTES=$(cat /proc/net/dev | grep wwan0 | awk {'print $2'} 2>/dev/null || true)
    update_json "WWanRxBytes_i" $ES_EIWWAN0RXBYTES
    ES_EIWWAN0RXPKTS=$(cat /proc/net/dev | grep wwan0 | awk {'print $3'} 2>/dev/null || true)
    update_json "WWanRxPkts_i" $ES_EIWWAN0RXPKTS
    ES_EIWWAN0TXBYTES=$(cat /proc/net/dev | grep wwan0 | awk {'print $10'} 2>/dev/null || true)
    update_json "WWanTxBytes_i" $ES_EIWWAN0TXBYTES
    ES_EIWWAN0TXPKTS=$(cat /proc/net/dev | grep wwan0 | awk {'print $11'} 2>/dev/null || true)
    update_json "WWanTxPkts_i" $ES_EIWWAN0TXPKTS

      # get gps data
    nmea=$(get_gps_value "NMEA_s")
    ES_GPSLAT=$(get_gps_value "Lat_f")
    ES_GPSLON=$(get_gps_value "Lon_f")

    update_json "Lat_f" $ES_GPSLAT
    update_json "Lon_f" $ES_GPSLON
    update_json "NMEA_s" $nmea

    ES_GPSALT=$(get_gps_value "Alt_f")
    update_json "Alt_f" $ES_GPSALT

    ES_GEOHASH=$(get_gps_value "GeoHash_s")
    update_json "GeoHash_s" $ES_GEOHASH

    # snr
    if [ -f /tmp/nextivity/snr ]; then
        snr=$(cat /tmp/nextivity/snr)
        update_json "ScSnr_f" $snr
    fi

    # get uptime value
    ES_PCUPTIME=$(cat /proc/uptime | awk '{print $1}' | awk -F . {'print $1'})
    update_json "UpTime_i" $ES_PCUPTIME

    # get ICCID - 8948032222084370091F
    if [ ! -f /tmp/nextivity/iccid ]; then
        touch /tmp/nextivity/iccid
    fi
    ES_ICCID=$(cat /tmp/nextivity/iccid)
    response=$(echo -e "AT+ICCID\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep "ERROR" )
    error=$(echo $response | grep "ERROR")
    if [ "$error" == "" ]; then
        # Get ICCID value if we havn't got it before
        if [ "$ES_ICCID" == "" -o "$ES_ICCID" == "SIM Missing" ]; then
	       ES_ICCID=$(echo -e "AT+ICCID\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep +ICCID: | awk {'print $2'})
            if [ "$ES_ICCID" != "" ]; then
                update_json "ICCID_s" $ES_ICCID
                echo $ES_ICCID > /tmp/nextivity/iccid
                gpio led green blink
            fi
        fi
        echo "ES_ICCID="$ES_ICCID
    elif [ "$ES_ICCID" == "" -o "$ES_ICCID" != "SIM Missing" ]; then
        update_json "ICCID_s" "SIM Missing"
        update_json "Connection_s" "Disconnected"
        echo "SIM Missing" > /tmp/nextivity/iccid
        echo "ES_ICCID="$error
        # led control
        gpio led red blink
        echo "SIM Missing" > /tmp/nextivity/conn_led_state
    fi

    # get IMEI - 015681000024927
    if [ ! -f /tmp/nextivity/imei ]; then
        touch /tmp/nextivity/imei
    fi

    imei=$(cat /tmp/nextivity/imei)

    if [ "$imei" == "" ]; then 
        ES_IMEI=$(echo -e "AT+CGSN\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep -Eo '[0-9]{15}')
        echo "ES_IMEI="$ES_IMEI
        if [ "$ES_IMEI" != "" ]; then
            update_json "IMEI_s" $ES_IMEI
            touch /tmp/nextivity/imei
            echo $ES_IMEI > /tmp/nextivity/imei
        fi
    fi

    # get IMSI - 260032008437009
    if [ ! -f /tmp/nextivity/imsi ]; then
        touch /tmp/nextivity/imsi
    fi

    imsi=$(cat /tmp/nextivity/imsi)

    if [ "$imsi" == "" ]; then
        ES_IMSI=$(echo -e "AT+CIMI\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep -Eo '[0-9]{15}')
        echo "ES_IMSI="$ES_IMSI
        if [ "$ES_IMSI" != "" ]; then
            update_json "IMSI_s" $ES_IMSI
            touch /tmp/nextivity/imsi
            echo $ES_IMSI > /tmp/nextivity/imsi
        fi
    fi

    # get revision identification - EM12AWPAR01A07M4G
    if [ ! -f /tmp/nextivity/modem_v ]; then
        touch /tmp/nextivity/modem_v
    fi

    modem_v=$(cat /tmp/nextivity/modem_v)

    if [ "$modem_v" == "" ]; then
        ES_MSWV=$(echo -e "AT+CGMR\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep 'M' | tail -n 1 | tr -d '\r')
        echo "ES_MSWV="$ES_MSWV
        if [ "$ES_MSWV" != "" ]; then
            update_json "ModemSwVersion_s" $ES_MSWV
            touch /tmp/nextivity/modem_v
            echo $ES_MSWV > /tmp/nextivity/modem_v
        fi
    fi

    # AT#RFSTS - Read Current Network Status
    # Bandwidth threshold for 5G to 5G+ - 100 uis a guess because I dont know what it is - Ask AT&T
    # JINHONG HO <jn4915@att.com> at AT&T says: The aggregated bandwidth to display "5G+" icon on NG-RAN @FR1 - <5G_Plus_BW_Threshold>
    # shall be set to 50MHz. The value shall be updatable parameter via FOTA.
    BW_THRESHOLD=50
    HPUE_STATE="OFF"
    NET_STAT=$(echo -e 'AT#RFSTS\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2 | grep "RFSTS:")
    modem_type=$(cat $wanstatus | awk -F':' '/Modem Type:/ {print $2}' | awk -F ';' '{print $1}')
    if [ "$NET_STAT" != "" ]; then
        echo $NET_STAT
        NUM_COMMA=$(echo $NET_STAT | tr -cd , | wc -c)
        echo $NUM_COMMA
        if [ "$NUM_COMMA" == "15" ]; then
            # LTE network
            update_json "NetMode_s" "LTE"
            BAND=$(echo $NET_STAT  | awk -F , {'print $16'} | tr -d '\r')
            update_json "ScFb_i" $BAND
            update_json "ScMode_s" "4G LTE"
            sed -i "s/Modem Type:$modem_type;/Modem Type:4GLTE;/g" $wanstatus
            ES_TXPWR=$(echo $NET_STAT | awk -F , {'print $8'})  
            update_json "TX_Power" $ES_TXPWR     
            if [ "$ES_TXPWR" -ge "23" ]; then       
                HPUE_STATE="ON"                                           
            fi

            ES_SCRSRP=$(echo $NET_STAT | awk -F , {'print $3'}) 
            update_json "ScRsrp_i" $ES_SCRSRP   
            ES_SCRSSI=$(echo $NET_STAT | awk -F , {'print $4'})
            update_json "ScRssi_i" $ES_SCRSSI      
            ES_SCRSRQ=$(echo $NET_STAT | awk -F , {'print $5'})
            update_json "ScRsrq_i" $ES_SCRSRQ 

        elif [ "$NUM_COMMA" == "24" ]; then
            # ENDC network
            update_json "NetMode_s" "EN-DC"
            ES_TXPWR=$(echo $NET_STAT | awk -F , {'print $8'})  
            update_json "TX_Power" $ES_TXPWR     
            BAND=$(echo $NET_STAT  | awk -F , {'print $22'} | tr -d '\r')
            update_json "ScFb_i" $BAND
            BANDWIDTH=$(echo $NET_STAT  | awk -F , {'print $23'})
            if [ "$BANDWIDTH" -gt "$BW_THRESHOLD" -a "$BANDWIDTH" -eg "77" ]; then
                update_json "ScMode_s" "5G+"
                sed -i "s/Modem Type:$modem_type;/Modem Type:5GPlus;/g" $wanstatus     
            else
                update_json "ScMode_s" "5G"
                sed -i "s/Modem Type:$modem_type;/Modem Type:5G;/g" $wanstatus                                                
            fi

            ES_SCRSRP=$(echo $NET_STAT | awk -F , {'print $3'}) 
            update_json "ScRsrp_i" $ES_SCRSRP   
            ES_SCRSSI=$(echo $NET_STAT | awk -F , {'print $4'})
            update_json "ScRssi_i" $ES_SCRSSI      
            ES_SCRSRQ=$(echo $NET_STAT | awk -F , {'print $5'})
            update_json "ScRsrq_i" $ES_SCRSRQ 

        elif [ "$NUM_COMMA" == "10" ]; then
            ES_TXPWR=$(echo $NET_STAT | awk -F , {'print $10'})  
            update_json "TX_Power" $ES_TXPWR     
            # NR network
            update_json "NetMode_s" "NR"
            BAND=$(echo $NET_STAT  | awk -F , {'print $7'} | tr -d '\r')
            update_json "ScFb_i" $BAND
            BANDWIDTH=$(echo $NET_STAT  | awk -F , {'print $8'})
            if [ "$BANDWIDTH" -gt "$BW_THRESHOLD" -a "$BANDWIDTH" -eg "77" ]; then
                update_json "ScMode_s" "5G+"
                sed -i "s/Modem Type:$modem_type;/Modem Type:5GPlus;/g" $wanstatus     
            else
                update_json "ScMode_s" "5G"
                sed -i "s/Modem Type:$modem_type;/Modem Type:5G;/g" $wanstatus                                                
            fi

            ES_SCRSRP=$(echo $NET_STAT | awk -F , {'print $4'}) 
            update_json "ScRsrp_i" $ES_SCRSRP   
            ES_SCRSSI=$(echo $NET_STAT | awk -F , {'print $5'})
            update_json "ScRssi_i" $ES_SCRSSI      
            ES_SCRSRQ=$(echo $NET_STAT | awk -F , {'print $6'})
            update_json "ScRsrq_i" $ES_SCRSRQ 

        fi
        echo $BAND

        signal_level=$(cat $wanstatus | awk -F':' '/Signal Level:/ {print $2}' | awk -F ';' '{print $1}')
            
        if [ "$ES_SCRSRP" -gt "-100" ]; then
            ES_SIGNALPERCENT="90"
            sed -i "s/Signal Level:$signal_level;/Signal Level:Excellent Signal;/g" $wanstatus             
        elif [ "$ES_SCRSRP" -gt "-110" ]; then
            ES_SIGNALPERCENT="80"
            sed -i "s/Signal Level:$signal_level;/Signal Level:Good Signal;/g" $wanstatus
            elif [ "$ES_SCRSRP" -gt "-120" ]; then
            ES_SIGNALPERCENT="70"
            sed -i "s/Signal Level:$signal_level;/Signal Level:Good Signal;/g" $wanstatus
        elif [ "$ES_SCRSRP" -gt "-125" ]; then
            ES_SIGNALPERCENT="50"
                sed -i "s/Signal Level:$signal_level;/Signal Level:Low Signal;/g" $wanstatus
        elif [ "$ES_SCRSRP" -le "-125" ]; then
            ES_SIGNALPERCENT="30"
            sed -i "s/Signal Level:$signal_level;/Signal Level:Bad Signal;/g" $wanstatus
        else
            ES_SIGNALPERCENT="0"
            sed -i "s/Signal Level:$signal_level;/Signal Level:No Signal;/g" $wanstatus
        fi
        update_json "SignalPercent_s" $ES_SIGNALPERCENT"%"
        echo "ES_SIGNALPERCENT:"$ES_SIGNALPERCENT
     fi
    echo "$HPUE_STATE" > /tmp/nextivity/hpue_onoff

    # get home network name
    if [ ! -f /tmp/nextivity/home_network ]; then
        touch /tmp/nextivity/home_network
    fi

    home_network=$(cat /tmp/nextivity/home_network)   

    operator_selection=$(echo -e "AT+COPS?\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep "+COPS")
    ###   +COPS: 0,1,"Orange",13
    if [ "$home_network" == "" ]; then
        ES_HNNAME=$(echo $operator_selection | awk -F ',' '{print $3}')
        ES_HNNAME=${ES_HNNAME//\"/}
        # In the case of AT&T remove '&' to get ATT
        ES_HNNAME=${ES_HNNAME//\&/}
        if [ "$ES_HNNAME" != "" ]; then
            update_json "HnName_s" $ES_HNNAME
            home_network=$ES_HNNAME
            touch /tmp/nextivity/home_network
            echo $ES_HNNAME > /tmp/nextivity/home_network
        fi
    fi

    # Get phone number
    if [ ! -f /tmp/nextivity/phone_number ]; then
        touch /tmp/nextivity/phone_number
    fi

    phone_number=$(cat /tmp/nextivity/phone_number)

    if [ "$phone_number" == "" ]; then
        ES_PHONENUMBER=$(echo -e "AT+CNUM\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep "+CNUM:" | awk -F'"' '{print $2}')
        length=${#ES_PHONENUMBER}
        if [ ${length} == "11" ] || [ ${length} == "12" ]; then
            if [ ${ES_PHONENUMBER:0:2} == "+1" ]; then
                cut_phone_number=${ES_PHONENUMBER:2}
                #echo $cut_phone_number
                ES_PHONENUMBER="${cut_phone_number:0:3}.${cut_phone_number:3:3}.${cut_phone_number:6:4}"
                #echo $formatted_number
            elif [ ${ES_PHONENUMBER:0:1} == "1" ]; then
                cut_phone_number=${ES_PHONENUMBER:1}
                #echo $cut_phone_number
                ES_PHONENUMBER="${cut_phone_number:0:3}.${cut_phone_number:3:3}.${cut_phone_number:6:4}"
                #echo $formatted_number
            fi
        fi
        echo "ES_PHONENUMBER="$ES_PHONENUMBER
        if [ "$ES_PHONENUMBER" != "" ]; then
            update_json "PhoneNumber_s" $ES_PHONENUMBER
            touch /tmp/nextivity/phone_number
            echo $ES_PHONENUMBER > /tmp/nextivity/phone_number
        fi
    fi

    # get Serving Cell Info
    servingcell=$(echo -e "AT#LTEDS\r\n" | picocom -b $baudrate -qx $wait_ms /dev/ttyUSB2 | grep "#LTEDS:")

    if [ "$servingcell" != "" ]; then    
        # connection status
        is_sim_detected=""
        if [ -f /tmp/nextivity/iccid ]; then
            is_sim_detected=$(cat /tmp/nextivity/iccid)
        fi
        if [ "$is_sim_detected" != "" -a "$is_sim_detected" != "SIM Missing" ]; then
            ES_CONNECTION=$(echo $servingcell | awk -F , {'print $15'} | awk -F "/" {'print $2'})
            #echo "ES_CONNECTION="$ES_CONNECTION
            update_connection_status $ES_CONNECTION
        fi     

        # mcc - 260
        ES_SCMCC=$(echo $servingcell | awk -F , {'print $4'} | awk {'print $1'})
        #update_json "ScMcc_i" $ES_SCMCC
        update_json "ScMcc_i" $ES_SCMCC
        # mnc  - 03
        ES_SCMNC=$(echo $servingcell | awk -F , {'print $4'} | awk {'print $2'})
        #update_json "ScMnc_i" $ES_SCMNC
        update_json "ScMnc_i" $ES_SCMNC
        
        # cellid - 44D5315 = 72176396
        ES_SCID=$(echo $servingcell | awk -F , {'print $6'} | awk -F "(" {'print $1'})
        if [ "$ES_SCID" != "" ]; then
            update_json "ScId_i" $ES_SCID
        fi
        
        # pcid - 162
        ES_SCPID=$(echo $servingcell | awk -F , {'print $6'} | awk -F "(" {'print $2'} | tr -d ')')
        update_json "ScPId_i" $ES_SCPID
                    
        # earfcn - 75          
        ES_SCEARFCN=$(echo $servingcell | awk  {'print $2'} | awk -F "/" {'print $1'})
        update_json "ScEarfcn_i" $ES_SCEARFCN            

        # tac - E2AE = 58030
        ES_SCTAC=$(echo $servingcell | awk -F , {'print $5'})
        update_json "ScTac_i" $ES_SCTAC

            # sinr = 174
        ES_SCSINR=$(echo $servingcell | awk -F , {'print $17'})
        res=$(is_number "$ES_SCSINR")
        if [ "$res" == "1" ]; then
            ES_SCSINR=$(( -200 + ($ES_SCSINR*10) / 5 ))  
            int_part=$(( ES_SCSINR / 10 ))
            fract_part=$(( $ES_SCSINR - $int_part*10 ))   
            ES_SCSINR=$int_part,$fract_part    
            update_json "ScSinr_f" $ES_SCSINR   
        fi    
    fi

    # get temp
    temp=$(echo -e "AT#TEMPSENS=2\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep "#TEMPSENS:" > /tmp/nextivity/temp)

    if [ "cat /tmp/nextivity/temp" != "" ]; then
        ES_TEMPPA1=$(cat /tmp/nextivity/temp | grep "PA_THERM"| awk -F ',' {'print $2'} | tr -d '\r')
        update_json "TPA1_i" $ES_TEMPPA1

        ES_TEMPS0=$(cat /tmp/nextivity/temp | grep "TSENS"| awk -F ',' {'print $2'} | tr -d '\r')
        update_json "TS0_i" $ES_TEMPS0
    fi

    echo "-------------------------"
}

check_modem_state() {

    if [ ! -f /tmp/nextivity/modem_gps_check ]; then
        # Make sure that the GPS is turned off
        # From AT command manual for modem:
        #   AT$GPSLOCK=<mode>
        #   0 GNSS Unlock
        #   1 Mobile-Initiated (MI) session is locked
        #   2 Mobile-Terminated (MT) session is locked
        #   3 Except for an emergency call, all (MI and MT) is locked
        CURR_GPS_LOCK=$(echo -e 'AT$GPSLOCK?\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2 | awk -F : {'print $2'})
        if [ "${CURR_GPS_LOCK}" != "" ]; then
            touch /tmp/nextivity/modem_gps_check
            if [ "${CURR_GPS_LOCK}" -ne 3 ]; then
                echo "GPS is ON! - Turn it off!"
                echo -e 'AT$GPSLOCK=3\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2
                # Set GPS to come up disabled at power up
                echo -e 'AT$GNSSDISACFG=5\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2
            fi
        fi
    fi

    # Should we do a power class check?
    check_pc=0
    if [ ! -f /tmp/nextivity/last_pc_check ]; then
        # Power class check was never done this power cycle - do one now
        check_pc=1
    else
        # Do a power class check every 5 minutes to allow for SIM card swap
		last_check=$(cat /tmp/nextivity/last_pc_check)
		now=$(date +%s)
		diff=$(($now - $last_check))
		if [ "$diff" -ge "300" ]; then
			check_pc=1
		fi        
    fi

    if [ "$check_pc" == "1" ]; then
        # Ensure that Power Class 1 (+31dBm) is always enabled for the Telit Module
        CURR_PWR_CLS=$(echo -e 'AT#PC1ENA?\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2 | awk -F : {'print $2'})
        echo $CURR_PWR_CLS
        if [ "${CURR_PWR_CLS}" != "" ]; then
            # record that power class check has been done and the time.
            now=$(date +%s)
            echo $now > /tmp/nextivity/last_pc_check
            if [ "$(echo "$CURR_PWR_CLS" | tr -d ' \r\n')" != "1,F" ]; then
                echo "Not PowerClass 1! - Set it to 1!"
                echo -e 'AT#PC1ENA=1\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2
				# resetting the modem
				echo -e 'AT#REBOOT\r\n' | picocom -qx 2000 -b 115200 /dev/ttyUSB2
                rm /tmp/nextivity/last_pc_check
                rm /tmp/nextivity/modem_gps_check
				sleep 15
            fi
        fi
    fi
}

check_modem_state
get_status_to_json


# $1 - prev_time value, $2 - diff time
# return 0 - if no necessary diff, 1 - if yes
#check_diff_time() {
#  res=0
#  prev_time=$1
#  cur_time=$(date +%s 2>/dev/null || true)
#  diff=$(( $cur_time - $prev_time ))
#  if [ "$diff" -ge $2 ]; then
#      let res=1
#  fi

#  echo $res
#}

#prev_time=0

#while true; do
#    now=$(date "+%s")

#    diff=$(($now - $prev_time))
#    delta=15
#    if [ "$diff" -ge $delta ]; then
#      prev_time=$now
#      get_status_to_json
#    fi
#done
