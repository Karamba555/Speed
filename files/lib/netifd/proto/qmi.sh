#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

message_out() {
  message=$@
  echo $message
  logger "qmi.sh: $message"
}

# Check we aren't in MBIM mode
qmi_check() {
    proto_path=/tmp/nextivity/protocol
    port=/dev/ttyUSB2
    wait_ms=10
    baudrate=115200

    proto=$(cat $proto_path 2>/dev/null || true)
    if [ "$proto" != "qmi" ]; then
        module=qmi_wwan.ko
        module_path=/lib/modules/$(uname -r)/$module

        if ! [ -e $module_path ] ; then
            ln -s /usr/lib/awc/$module $module_path
        fi

        # check if qmi_wwan driver was loaded
        if ! lsmod | grep qmi_wwan > /dev/null; then
            logger -t qmi.sh "insert qmi_wwan, reboot modem"
            insmod $module 2>/dev/null || true
            /sbin/awc-usb-reset.sh
            sleep 10
        fi

        cnt=0
        while true; do
            mode=$(echo -e 'AT+QCFG="usbnet"\r\n' | picocom -b $baudrate -qx $wait_ms $port | grep "+QCFG:" | awk -F , {'print $2'} 2>/dev/null || true)
            mode=${mode:0:1}
            #logger -t "qmi.sh" "res:$res"
            if [ "$mode" != "" ];then 
                break
            fi

            if [ "$cnt" -ge "20" ]; then
                break
            fi

            cnt="$((cnt + 1))"

            sleep 1
        done

        mode=$(echo -e 'AT+QCFG="usbnet"\r\n' | picocom -b $baudrate -qx $wait_ms $port | grep "+QCFG:" | awk -F , {'print $2'} 2>/dev/null || true)
        mode=${mode:0:1}
        logger -t qmi.sh "mode: $mode"
        if [ "$mode" == 0 ] ; then
            message_out "Correctly in QMI mode"
            echo "qmi" > $proto_path
        elif [ "$mode" == 2 ] ; then
            message_out "In MBIM mode, switching to QMI"
            echo "mbim" > $proto_path
            echo -e 'AT+QCFG="usbnet",0\r\n' | picocom -b $baudrate -qx $wait_ms $port
        else
            message_out "unknown"
            echo "unknown" > $proto_path
        fi
    fi
}

quectel_cm_start() {
  if ! pidof quectel-CM 1>/dev/null; then
    # Check we are in QMI mode
#     qmi_check

#      auto=$(qmicli -p --silent --wds-get-autoconnect-settings -d /dev/cdc-wdm0 2>/dev/null | grep Status | cut -d \' -f 2)
#      if [ -n "$auto" -a "$auto" != "disabled" ] ; then
#        message_out "Disabling auto-connect mode"

#        qmicli -p --wds-set-autoconnect-settings=disabled -d /dev/cdc-wdm0
#        /sbin/awc-usb-reset.sh
#        sleep 10
#        continue
#      fi

      pin=$(uci get network.wwan.pin 2>/dev/null || true)
      apn=$(uci get network.wwan.apn 2>/dev/null || true)

      args=

      if [ -n "$apn" ] ; then
        args="-s $apn"
      fi

      if [ -n "$pin" ] ; then
        args="$args -p $pin"
      fi

      args="$args -f /tmp/nextivity/quectel-cm-log"
   
      message_out "Nextivity quectel-CM starting"
      /usr/bin/quectel-CM $args &

      conn_state=$(cat /tmp/nextivity/conn_led_state)
      cur_addr=""

      if [ "$conn_state" != "SIM Missing" ]; then
          # wait for getting IP-address
          cnt=0
          while true; do
              cur_addr=$(ifconfig wwan0 | grep "inet addr" | awk -F : {'print $2'} | awk {'print $1'} 2>/dev/null || true)
              if [ "$cur_addr" != "" ]; then
                  logger -t qmi.sh "Has got IP: $cur_addr"
                  break
              fi

              if [ "$cnt" -ge "20" ]; then
                  break
              fi

              cnt="$((cnt + 1))"

              sleep 1
          done
      fi

      if [ "$cur_addr" == "" -a "$conn_state" != "SIM Missing" ]; then
          logger -t qmi.sh "Has not got IP - killall quectel-CM"
          killall quectel-CM 2>/dev/null || true
      else
          # Nameservers
          nameservers="8.8.8.8 8.8.4.4"

          echo -n > /etc/resolv.conf
          for nameserver in $nameservers; do
              echo "nameserver $nameserver" >> /etc/resolv.conf
              logger -t qmi.sh "nameserver: $nameserver"
          done
 
          res=$(cat /etc/resolv.conf 2>/dev/null || true)
          logger -t qmi.sh "resolv.conf: $res"

          # update firewall rules
#          nft flush ruleset
#          nft -f /etc/nftables.rules
#          fw3 reload
      fi
#    fi
  fi
}

proto_qmi_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_int delay
	proto_config_add_string modes
	proto_config_add_string pdptype
	proto_config_add_int profile
	proto_config_add_boolean dhcp
	proto_config_add_boolean dhcpv6
	proto_config_add_boolean autoconnect
	proto_config_add_int plmn
	proto_config_add_int timeout
	proto_config_add_int mtu
	proto_config_add_defaults
}

proto_qmi_setup() {
	local interface="$1"
	local dataformat connstat plmn_mode mcc mnc
    local device apn delay modes pdptype
	local profile dhcp dhcpv6 autoconnect plmn timeout mtu $PROTO_DEFAULT_OPTIONS
	local ip4table ip6table
	local cid_4 pdh_4 cid_6 pdh_6
	local ip_6 ip_prefix_length gateway_6 dns1_6 dns2_6

    json_get_vars device apn delay modes
	json_get_vars pdptype profile dhcp dhcpv6 autoconnect plmn ip4table
	json_get_vars ip6table timeout mtu $PROTO_DEFAULT_OPTIONS
	
    if [ ! -d /tmp/nextivity ]; then
    	mkdir /tmp/nextivity
    fi

    touch /tmp/nextivity/qmi_script_log
    touch /tmp/nextivity/quectel-cm-log
 
    qmi_check
 
	[ "$timeout" = "" ] && timeout="10"

	[ "$metric" = "" ] && metric="0"

	[ -n "$ctl_device" ] && device=$ctl_device

	[ -n "$device" ] || {
		echo "No control device specified"
        logger -t "qmi.sh" "1.device"$device
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	device="$(readlink -f $device)"
	[ -c "$device" ] || {
		echo "The specified control device does not exist"
        logger -t "qmi.sh" "2.device"$device
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	devname="$(basename "$device")"
	devpath="$(readlink -f /sys/class/usbmisc/$devname/device/)"
	ifname="$( ls "$devpath"/net )"
        logger -t "qmi.sh" "devname="$devname
        logger -t "qmi.sh" "devpath="$devpath
        logger -t "qmi.sh" "ifname="$ifname
	[ -n "$ifname" ] || {
		echo "The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$mtu" ] && {
		echo "Setting MTU to $mtu"
		/sbin/ip link set dev $ifname mtu $mtu
	}

	
	echo "Setting up $ifname"
	proto_init_update "$ifname" 1
	proto_set_keep 1
	proto_send_update "$interface"

	#local zone="$(fw3 -q network "$interface" 2>/dev/null)"

    killall quectel-CM

#    if [ ! -e /tmp/nextivity/check_modem_update ]; then
#        /sbin/update_modem.sh
#        touch /tmp/nextivity/check_modem_update
#    fi

    quectel_cm_start

    proto_init_update "$ifname" 1 
    proto_set_keep 1       
    ip=$(ifconfig wwan0 | grep "inet addr" | awk {'print $2'} | awk -F ":" {'print $2'} 2>/dev/null)
#    subnet=$(ip -f inet -o a s scope global wwan0 | grep -Po 'inet \S+' | sed 's#.*/\(.*\)#\1#' 2>/dev/null || true)                                            
    subnet=$(ip -f inet -o a s scope global wwan0 | awk '{print $4}' | sed 's#.*/##' 2>/dev/null)
    proto_add_ipv4_address "$ip" "$subnet"
    proto_send_update "$interface"

#    if [ "$ip" != "" ]; then
#        echo $ip > /tmp/nextivity/mobile_ip
#    fi

    rm /tmp/nextivity/qmi_script_log
}

qmi_wds_stop() {
	local cid="$1"
	local pdh="$2"
}

proto_qmi_teardown() {
	local interface="$1"

	local device cid_4 pdh_4 cid_6 pdh_6
	json_get_vars device
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol qmi
}
