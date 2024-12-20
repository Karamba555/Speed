#!/bin/sh -e

# Enable NAT vs Passthrough mode for Nextivity routers

command=$1

logger -t awc-set-mode.sh "command: $command"

proto=$(uci get network.wwan.proto 2>/dev/null || true)

# Check old state due to possible uci change
if [ "$1" == "-c" ] ; then
  check=true
  shift
else
  check=false
fi

if [ "$1" == "-f" ] || [ "$2" == "-f" ] ; then
  force=true
else
  force=false
fi


current_mode=$(uci get awc.main.bridge_mode 2>/dev/null || true)

logger -t awc-set-mode.sh "current_mode: $current_mode"
 
# Process check only
if  [ "$1" == "-p" ] ; then
  process=true
  command=$current_mode
  logger -t awc-set-mode.sh "command: $command"
else
  process=false
fi


if $check; then
  status=$(cat /tmp/nextivity/bridge_mode 2>/dev/null || true)

  if [ -z "$status" ] ; then
    status=false
  fi

  #logger "Passthrough mode: old status is $status"

  command=$current_mode

else
  if [ -z "$current_mode" -o "$current_mode" == "off" ] ; then
    status=false
  else
    status=true
  fi

  if $force && [ "$command" == "-f" ] ; then
    command=$status
  fi
fi

if [ "$command" == "enable" -o "$command" == "1" -o \
     "$command" == "true" -o "$command" == "bridge" -o \
     "$command" == "on" ] ; then

  if ! $force && $status && ! $process; then
    echo "Passthrough mode already set"
    logger -t awc-set-mode.sh "Passthrough mode already set"
    exit 0
  fi

  mode=bridge

elif [ "$command" == "disable" -o "$command" == "0" -o \
       "$command" == "false" -o "$command" == "nat" -o \
       "$command" == "off" ] ; then
  if ! $force && ! $status && ! $process; then
    echo "NAT mode already set"
    logger -t awc-set-mode.sh "NAT mode already set"
    exit 0
  fi

  mode=nat

elif [ "$command" == "status" ] ; then
  logger -t awc-set-mode.sh "command: $command"
  if $status; then
    echo "Passthrough mode enabled"
  else
    echo "NAT mode enabled"
  fi
  exit 0

else
  echo "Usage: $0 [enable|disable]" 1>&2
  logger -t awc-set-mode.sh "Usage: $0"
  exit 1

fi


if $force; then
  logger "Nextivity Passthrough mode set to $enable"
fi

awc_disable_service() {
  service=$1

  if find /etc/rc.d/ | grep -q $service; then
    message="Nextivity Passthrough Mode: Stop/Disable $service"
    echo $message || true
    $force && logger $message
    /etc/init.d/$service stop 2>/dev/null    || true
	/etc/init.d/$service disable 2>/dev/null || true
  fi
}


awc_enable_service() {
  service=$1

  if ! find /etc/rc.d/ | grep -q $service; then
    message="Nextivity Passthrough Mode: Start/Enable $service"
    echo $message || true
    $force && logger $message
    /etc/init.d/$service enable 2>/dev/null || true
	/etc/init.d/$service start 2>/dev/null  || true
  fi
}

board=$(grep "Target board" /etc/nextivity_build_info | cut -d ' ' -f 3)
logger -t awc-set-mode.sh "board: $board"
hnet=/etc/hotplug.d/net/25-modemmanager-net
htty=/etc/hotplug.d/tty/25-modemmanager-tty
husb=/etc/hotplug.d/usb/25-modemmanager-usb

all_ports="lan1 lan2"

for port in ${all_ports}; do
  uci del_list network.@device[0].ports=${port}
done

if [ "$board" = "fi" ] || [ "$board" = "megafi" ] ; then
  for port in ${all_ports}; do
    uci del_list network.@device[1].ports=${port}
  done
fi
logger -t awc-set-mode.sh "mode: $mode"
if [ "$mode" == "bridge" ] ; then
  if ! $process; then
    message="Setting Passthrough mode"
    echo $message || true
    $force && logger $message
  fi

  # Network configuration
  uci set awc.main.bridge_mode=on
  
  uci set network.@device[0].ports='lan1'

  uci set network.wwan.proto=qmi
  uci set network.wwan.device='/dev/cdc-wdm0'

  # MTU for the ethernet interface must be 1430 or greater, or this triggers
  # a bug on RX packets
  uci set network.lan.mtu=1430
  
  # Set the DHCP lease limit 
  uci set dhcp.lan.limit='1'

  # Set DNS server
  server=$(uci get dhcp.@dnsmasq[0].server 2>/dev/null || true)
  if [ "$server" == "" ]; then
      uci add_list dhcp.cfg01411c.server='8.8.8.8'
      uci add_list dhcp.cfg01411c.server='8.8.4.4'
  fi

  # Enable services
  awc_enable_service dnsmasq
  awc_enable_service odhcpd
  awc_enable_service sysntpd
  
#  awc_disable_service modemmanager

  rm -f $hnet
  rm -f $htty
  rm -f $husb
  
elif [ "$mode" == "nat" ] ; then
  if ! $process; then
    message="Setting NAT mode"
    echo $message || true
    $force && logger $message
  fi

  # Network configuration
  uci set awc.main.bridge_mode=off

  port_mode=$(uci get network.wan.device)

  # if port LAN2 was set as wan in the settings
  if [ "$port_mode" == "wan" ]; then
      uci set network.@device[0].ports='lan1'
  # if port LAN2 was set as lan in the settings    
  else 
      uci set network.@device[0].ports='lan1 wan'
  fi
  uci set network.wwan.proto=qmi
  uci set network.wwan.device='/dev/cdc-wdm0'

  # Set DNS server
  server=$(uci get dhcp.@dnsmasq[0].server 2>/dev/null || true)
  if [ "$server" == "" ]; then
      uci add_list dhcp.cfg01411c.server='8.8.8.8'
      uci add_list dhcp.cfg01411c.server='8.8.4.4'
  fi

  # Reset MTU to default of 1342 which works in NAT mode
  uci set network.lan.mtu=1342
  
  # Set the DHCP lease limit
  uci set dhcp.lan.limit='100'

  # Enable services
  awc_enable_service dnsmasq
  awc_enable_service odhcpd
  awc_enable_service sysntpd
  
  awc_disable_service modemmanager

  rm -f $hnet
  rm -f $htty
  rm -f $husb

fi

uci commit

if [ "$proto" != "qmi" ]; then
    service network restart
fi

if $process; then
  exit 0
fi

message="Nextivity Rebooting"
echo $message || true
$force && logger $message

/bin/sync
/sbin/reboot
