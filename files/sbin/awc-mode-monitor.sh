#!/bin/sh -e

message_out() {
  message=$@
  echo $message
  logger "WWAN Check: $message"
}

cp /etc/modemstatus.json /tmp/nextivity

# wait while qmi.sh is running
while true; do
    if [ ! -e /tmp/nextivity/qmi_script_log ]; then
        break
    fi
done

status_prev_time=0
status_delta=15
apn_prev_time=0
apn_delta=5
last_apn_config=$(uci get network.wwan.apn 2>/dev/null || true)

band_lock_prev_time=0
band_lock_delta=5
default_band="2000000003000281a"
b14_only_band="2000"
config_band_lock=$(uci get awc.main.band_lock 2>/dev/null || true) # band locking need to be set to 'Default' at startup
if [ "$config_band_lock" != "-1" ]; then                           # therefore awc.main.band_lock should be set to '-1'
    uci set awc.main.band_lock='-1'
    uci commit awc
fi
#band=$(echo -e 'AT+QCFG="band"\r\n' | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep "+QCFG:" | awk -F, '{gsub(/^0x/, "", $3); print $3}' 2>/dev/null || true)
#if [ "$band" != "$default_band" ]; then
#    last_band_lock_config='0'
#else
#    last_band_lock_config='-1'
#fi
#logger -t awc-mode-monitor.sh "band: $band"

echo '{"state": "Disabled", "time": ""}' > /tmp/nextivity/post_state.json
touch /tmp/nextivity/cur_wan_ip
echo -1 > /tmp/nextivity/signal_percent
touch /tmp/nextivity/c0rxpower
touch /tmp/nextivity/c0ecio
touch /tmp/nextivity/c0rsrp
touch /tmp/nextivity/c0phase
touch /tmp/nextivity/c1rxpower
touch /tmp/nextivity/c1ecio
touch /tmp/nextivity/c1rsrp
touch /tmp/nextivity/c1phase
touch /tmp/nextivity/txpower
touch /tmp/nextivity/snr
echo "" > /tmp/nextivity/iccid
echo "" > /tmp/nextivity/imei
echo "" > /tmp/nextivity/imsi
echo "" > /tmp/nextivity/modem_v
echo "" > /tmp/nextivity/home_network
echo "" > /tmp/nextivity/conn_led_state
echo "" > /tmp/nextivity/tx_power_led_state
echo "0" > /tmp/nextivity/passthrough_is_switched
uci get network.lan.ipaddr > /tmp/nextivity/cur_lan_ip
wwan_timeout=/tmp/nextivity/wwan_timeout
wwan_timeout_log=/tmp/nextivity/wwan_timeout_log
cp /etc/wan_status /tmp/wan_status

while true; do
    # WWAN timeout monitor, which watches kernel logs for a bad USB/Modem state
: << 'COMMENT'
    if [ -e "$wwan_timeout" ] ; then
        message_out "Driver stuck state detected - resetting connection"
        rm -f $wwan_timeout

        date=$(date)
        servingcell=$(echo -e "AT+QENG=\"servingcell\"\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep +QENG: 2>/dev/null || true)
        echo $date" "$servingcell >> $wwan_timeout_log
        qcainfo=$(echo -e "AT+QCAINFO\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep +QCAINFO: 2>/dev/null || true)
        echo $date" "$qcainfo >> $wwan_timeout_log
        csq=$(echo -e "AT+CSQ\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep +CSQ: 2>/dev/null || true)
        echo $date" "$csq >> $wwan_timeout_log
        creg=$(echo -e "AT+CREG?\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep +CREG: 2>/dev/null || true)
        echo $date" "$creg >> $wwan_timeout_log    
        cereg=$(echo -e "AT+CEREG?\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep +CEREG: 2>/dev/null || true)
        echo $date" "$cereg >> $wwan_timeout_log
        cops=$(echo -e "AT+COPS?\r\n" | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep +COPS: 2>/dev/null || true)
        echo $date" "$cops >> $wwan_timeout_log        


        echo "---------------------------------------------------" >> $wwan_timeout_log

        # Stop quectel-CM to restart it below through $status_delta time
        killall quectel-CM 2>/dev/null || true
        /sbin/awc-usb-reset.sh
        sleep 8
        #status_delta=1
    fi
COMMENT
    # if passthrough mode
    mode=$(uci get awc.main.bridge_mode 2>/dev/null || true)
    if [ "$mode" == "on" ]; then
        # if wwan interface is up
        #is_wwan_up=$(ifconfig | grep wwan0 2>/dev/null || true)
        is_wwan_up=$(ifconfig wwan0 | grep "inet addr" | awk -F : {'print $2'} | awk {'print $1'} 2>/dev/null || true)
        if [ "$is_wwan_up" != "" ]; then
            # check if qmi.sh isn't running
            if [ ! -e /tmp/nextivity/qmi_script_log ]; then
                sleep 3

                cut_wwan_addr=""
                cut_lan_addr=""

                cur_lan_addr=$(uci get network.lan.ipaddr 2>/dev/null || true)
                prev_lan_addr=$(cat /tmp/nextivity/cur_lan_ip)
                cur_addr=$(cat /sys/class/net/wwan0/qmi/bridge_ipv4 2>/dev/null || true)
                # for the first time
                if [ "$cur_addr" == "" -o "$cur_addr" == "0.0.0.0" ];then
                    cur_addr=$(ifconfig wwan0 | grep "inet addr" | awk -F : {'print $2'} | awk {'print $1'} 2>/dev/null || true)  
                    mobile_ip=$cur_addr
                    echo $cur_addr > /tmp/nextivity/mobile_ip
                    /sbin/passthrough.sh
                    message_out "cur_addr: $cur_addr"
                # if the modem has got new IP
                else
                    mobile_ip=$(cat /tmp/nextivity/mobile_ip)
                fi 
                # if LAN_IP was changed or the modem has got new IP - it's necessary to reload passthrough mode
                if [ "$cur_lan_addr" != "$prev_lan_addr" -o "$cur_addr" != "$mobile_ip" ]; then
                    message_out "cur_addr: $cur_addr, mobile_ip: $mobile_ip"
                    #if pidof quectel-CM 1>/dev/null; then
                        res=$(cat /tmp/nextivity/passthrough_is_switched)
                        message_out "res $res"
                        if [ "$res" == "1" ]; then
                            # switch off passthrough mode
                            ifconfig br-lan:0 down 2>/dev/null || true
                            ethtool -r lan1 2>/dev/null || true
                            wifi 2>/dev/null || true
                            sleep 1
                            killall quectel-CM 2>/dev/null || true
                            ifdown wwan && ifup wwan 
                            echo "0" > /tmp/nextivity/passthrough_is_switched
                            message_out "switch off passthrough mode" 
                        else
                            # switch on passthrough mode
                            echo $cur_lan_addr > /tmp/nextivity/cur_lan_ip
                            /sbin/passthrough.sh
                            message_out "switch on passthrough mode"
                        fi
                    #fi
                fi
            fi
        fi
    fi

    now=$(date "+%s")

    diff=$(($now - $status_prev_time))

    # get data from the modem every $delta seconds
    if [ "$diff" -ge $status_delta ]; then
      status_prev_time=$now
      # if usb interface was detected
      if [ -e /dev/ttyUSB2 ]; then
          # check if qmi.sh isn't running
          if [ ! -e /tmp/nextivity/qmi_script_log ]; then
              # check if "quectel_cm" process is running
              if ! pidof quectel-CM 1>/dev/null; then
                  if [ ! -e /tmp/nextivity/qmi_script_log ]; then
                  ifdown wwan && ifup wwan
                  logger -t awc-mode-monitor.sh "quectel-CM stopped: trigger for restarting quectel-CM"
                  fi
              fi

              if [ -f /tmp/nextivity/modemstatus.json ]; then
                  # get data from the modem and put it to the "/tmp/nextivity/modemstatus.json"
                  /sbin/status_gather.sh 2>/dev/null || true
              fi
          fi
          status_delta=15
      else
          status_delta=1
      fi
    fi

    now=$(date "+%s")

    diff=$(($now - $apn_prev_time))
    # check apn every $delta seconds
    if [ "$diff" -ge $apn_delta ]; then
        apn_prev_time=$now
        # Check if the UCI APN setting has been updated
        config_apn=$(uci get network.wwan.apn 2>/dev/null || true)

        if [ "$config_apn" != "$last_apn_config" ] ; then
            # ifdown wwan automatically restart it in quectel_cm_start
      #      killall quectel-CM 2>/dev/null || true
            ifdown wwan && ifup wwan
            logger -t awc-mode-monitor.sh "apn changed: trigger to restart quectel-CM"
            status_prev_time=0
            last_apn_config=$config_apn
        fi
    fi
: << 'COMMENT'
    now=$(date "+%s")

    diff=$(($now - $band_lock_prev_time))
    # check band_lock every $band_lock_delta seconds
    if [ "$diff" -ge $band_lock_delta ]; then
        band_lock_prev_time=$now
        # Check if the UCI band_lock setting has been updated
        config_band_lock=$(uci get awc.main.band_lock 2>/dev/null || true)

        if [ "$config_band_lock" != "$last_band_lock_config" ] ; then
            # check if qmi.sh isn't running
            if [ ! -e /tmp/nextivity/qmi_script_log ]; then
                # set band according to the $config_band_lock value
                logger -t awc-mode-monitor.sh "set ltebandval: $config_band_lock"
                if [ "$config_band_lock" == "14" ];then
                    a="AT+QCFG=\"band\",260,"${b14_only_band}",0\r\n"
                else
                    a="AT+QCFG=\"band\",260,"${default_band}",0\r\n"
                fi
                echo -e $a | picocom -b 115200 -qx 10 /dev/ttyUSB2 2>/dev/null

                logger -t "qmi.sh" "band changed: trigger to restart quectel-CM"
                ifdown wwan && ifup wwan

                ltebandval=$(echo -e 'AT+QCFG="band"\r\n' | picocom -b 115200 -qx 10 /dev/ttyUSB2 | grep "+QCFG:" | awk -F, '{gsub(/^0x/, "", $3); print $3}' 2>/dev/null || true)
                if [ "$ltebandval" == "$b14_only_band" ]; then
                    ltebandval=14
                elif [ "$ltebandval" == "$default_band" ]; then
                    ltebandval=-1
                fi
                logger -t awc-mode-monitor.sh "get ltebandval: $ltebandval"
                if [ "$ltebandval" == "$config_band_lock" ]; then
                    last_band_lock_config=$config_band_lock
                fi
            fi
        fi
    fi
COMMENT
    sleep 1
done
