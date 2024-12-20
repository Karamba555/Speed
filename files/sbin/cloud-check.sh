#!/bin/sh -e

. /usr/share/libubox/jshn.sh

last_log_message=

message_out() {
  message="$@"

  if [ -z "$message" ] ; then return; fi
  
  if [ "$message" = "$last_log_message" ] ; then
    return
  fi

  last_log_message=$message

  echo "$message"
  logger "Nextivity Cloud Check: $message"
}


message_err() {
  message="$@"

  if [ -z "$message" ] ; then return; fi

  if [ "$message" = "$last_log_message" ] ; then
    return
  fi

  last_log_message=$message

  echo "$message" 1>&2
  logger "Nextivity Cloud Check Error: $message"
}

debug() {
  $verbose || return 0

  message="$@"

  echo "$message" 1>&2
}


message_out "Start"

cloud_state=/tmp/nextivity/cloud_state.json
registration_data=/etc/awc/user_registration
all_config=/tmp/nextivity/all-config.json
boardShort=$(grep Target /etc/nextivity_build_info | cut -d ' ' -f 3)

sha256sumSave=/etc/awc/sha256config


wakeup_reason=
wakeup_reason_file=/tmp/nextivity/wakeup-reason

cloud_status_json() {
  time=$1
  shift
  state=$@

  echo "{\"state\": \"$state\", \"time\": $time}" > $cloud_state
}


generate_json_all_config() {
  /sbin/ucitojson.sh -n "${boardShort}_config" -o > $all_config
  sha256sum $all_config | cut -f 1 -d ' '
}


set_cloud_status() {
  state="$@"
  now=$(date +%s)

  case $state in
    "Startup")
       if ! [ -e $cloud_state ] ; then
         cloud_status_json "$now" "$state"
       fi
       ;;

     *)
       cloud_status_json "$now" "$state"
       ;;

  esac
}


get_poll() {
  # Want to immediately poll to confirm the last action
  if $confirm_action; then
    echo 0
    return
  fi

  if [ -n "$override_poll" ] ; then
    echo "$override_poll"
    return
  fi

  poll=$(uci get awc_cloud.cloud.action_poll 2>/dev/null || true)

  if [ -z "$poll" ] || [ "$poll" -gt 86400 ] ; then
    poll=86400
  elif [ "$poll" -lt 60 ] ; then
    poll=60
  fi

  # Poll at least every 1 minute until a successful poll
  if ! $first_time && ! $first_success; then
    if [ $poll -gt 60 ] ; then
      poll=60
    fi
  fi
  
  echo $poll
}


# Sleep this for this many seconds, but allow to be woken up
sleep_wait() {
  delay=$1
  
  wakeup_reason=
  
  sleep "$delay" &
  pid=$!
  
  echo $pid > /tmp/nextivity/cloud-sleep.pid

  if ! wait $pid ; then
    message_out "Wakeup requested"
    wakeup_reason=$(cat $wakeup_reason_file 2>/dev/null || true)
    return 1
  fi
  
  return 0
}


modem_find() {
  modem_device=$(uci get network.wwan.device 2>/dev/null || true)

  # Must exist
  if [ -z "$modem_device" ] || ! [ -e "$modem_device" ] ; then
    modem_num=
    return 1
  fi

  # Determine ModemManager index
  modem_num=$(mmcli -L 2>/dev/null | grep -v "No modems" | cut -d '/' -f 6 | cut -d ' ' -f 1)

  if [ -z "$modem_num" ] ; then
    return 1
  fi
}



modem_connected() {
  if [ -z "$modem_num" ] ; then return 1; fi

  # Must be connected
  if ! mmcli -m "$modem_num" | grep "state:.*connected" > /dev/null; then
    return 1
  fi

  return 0
}


get_software_version() {
  software_version=$(grep -i "Firmware Version" /etc/nextivity_build_info | sed 's#.*: ##')

  if [ -z "$software_version" ] || [ "$software_version" = "Development Build" ] ; then
    software_version="unknown"
    suffix=""
  else
    suffix="_MEGAFI2"
  fi

  software_version=${software_version}${suffix}

  echo ${software_version}
}

# Perform fetch, and return response code, if any, or return 1 on
# error. The HTTP output is available in $fetch_output, although
# this should be empty at present

fetch() {
  host=$1
  query=$2

  if [ -z "$host" ] || [ -z "$query" ] ; then
    return 1
  fi

  shift 2

  json=$@

  if [ -n "$json" ] ; then
    path="Prod/register?${query}"
  else
    path="Prod?${query}"
  fi

  if ! echo "$host" | grep -q ://; then
    host=https://${host}
  fi

  url="${host}/${path}"

  rm -f "$fetch_output" "$fetch_headers"

  if [ -n "$json" ] ; then
    debug curl -s -v -H "'content-type: application/json'" \'"$url"\' -d \'"$json"\'
    curl -s -v -H "content-type: application/json" "$url" -d "$json" \
       -o "$fetch_output" 2>"$fetch_headers" || true
  else
    debug curl -s -v -H '"content-type: application/json"' \'"$url"\' -d \'"$json"\'
    curl -s -v -H "content-type: application/json" "$url" \
      -o "$fetch_output" 2>"$fetch_headers" || true
  fi

  if $verbose; then
    echo -n "Output:  " 1>&2 ; cat "$fetch_output"  1>&2
    echo -n "Headers: " 1>&2 ; cat "$fetch_headers" 1>&2
  fi
   
  http_code=$(grep -v "^> POST" "$fetch_headers" | grep "^< HTTP" | sed 's#.* HTTP/2 ##' | cut -d ' ' -f 1)
  
  if [ -z "$http_code" ] ; then
    curl_error=$(head -1 "$fetch_headers" | cut -b 3-)

    if [ -z "$curl_error" ] ; then
      http_code="Error: Timeout or empty response"
    else
      http_code="Error: $curl_error"
    fi
    
  fi

  if $verbose; then
    debug "Code: $http_code"
    debug -n "Output: '"

    if [ -e "$fetch_output" ] ; then
      cat "$fetch_output"        1>&2

      if grep -q "Object reference" "$fetch_output"; then 
        output=$(cat "$fetch_output")

        message_err "Unexpected reponse from cloud: '$output'"
      fi
  	   
    else
      echo "(Empty)" 1>&2
    fi

    debug -ne "'\nHeaders: '"

    if [ -e "$fetch_headers" ] ; then
      cat "$fetch_headers"       1>&2
    else
      echo "(Empty)" 1>&2
    fi

    debug -ne "'\n"
  fi
   
  echo "$http_code"
}


handle_response() {
  data=$@

  json_init

  # The JSON parsing library doesn't care for the '.' naming that we use due to
  # shell parsing issues.
  if ! json_load "$(echo "$data" | sed 's#.update"#_update"##g')"; then
    message_err "Failed to parse JSON response"
    return 1
  fi
  
  # Now we can unset flags from last time, since we have valid JSON
  report_full_config=false

  # TODO: May need to look at list of items, or array of results
  if json_select ei.action >/dev/null 2>/dev/null; then
    password=
    influx=
    config_modem_period=
    ssid=
    ipaddress=
    netmask=
    config_state=
  
    if json_get_var update "software_update"; then
      message_out "Update available: $data"
      #echo "$update" > $software_updates
      echo "$data" > "$software_updates"
      return 0

    elif json_get_var update "configuration_update"; then
      message_out "Configuration update received: $data"
      echo "$data" > "$config_updates"
      # Don't update action_sequence until we have successfully applied the config
      return 0

    elif json_get_var password "password_set"; then
      message_out "Setting Admin password"
      
      if [ -z "$password" ] ; then
        passwd -d root
      
      elif ! echo -e "$password\r$password" | passwd 1>/dev/null 2>/dev/null; then
        messsage_err "Failed to set password" 
        return 1
      fi
    
    elif json_get_var reset "password_reset"; then
      message_out "Admin password reset to factory"
      /sbin/password-reset.sh -f
	  
	elif json_get_var reboot_now "reboot_now"; then
      message_out "Rebooting now"
      reboot
	  
	elif json_get_var factory_reset "factory_reset"; then
      message_out "Factory reset pending"
      # We can't do the reset until after the action is confirmed
      do_factory_reset=true 
      
    elif json_get_var influx "influx"; then
      
      if [ "$influx" = "on" ]; then
        message_out "Influx push enabled"
        
      elif [ "$influx" = "off" ] ; then
        message_out "Inflush push disabled"
      
      else
        message_err "Unsupported Influx action: '$influx'"
        return 1
      
      fi 
      uci set awc.status_gather.logging_push="$influx"
      confirm_action=true
      
    elif json_get_var config_modem_period "get_modem_status"; then
    
      if [ "$config_modem_period" = "now" ] ; then
        :
      elif [ -n "$config_modem_period" ] && [ "$config_modem_period" -ge 0 ] ; then
        config_modem_report=$config_modem_period
        uci set awc_cloud.cloud.modem_report="$config_modem_period"
        uci commit
      else
        message_err "Get modem status: Invalid period: '$config_modem_period'"
        return 1
      fi

      message_out "Get modem status: Period set to $config_modem_period"
      
      confirm_action=true
      modem_report_period="$config_modem_period"

    elif json_select "api" >/dev/null 2>/dev/null; then 
 
      state=
      name=
 
      if json_get_var state "modem-status"; then
        name=status
  
      elif json_get_var state "power-cycle"; then
        name=power_cycle
        
      elif json_get_var state "reboot"; then
        name=reboot
        
      else 
        message_err "Unsupported API specified: '$data'"
        return 1
      fi
      
      if [ "$state" != "on" ] && [ "$state" != "off" ] ; then
        message_err "Unsupported API state specified for $name: '$state'"
        return 1
        
      fi
      
      message_out "Setting API for $name to $state"
      uci set awc.api.$name=$state
      confirm_action=true      
            
    elif json_get_var config_report "config_report" ; then
#      message_out "Report full configuration" 
       
      json_get_var state "state" 2>/dev/null || true
            
      if [ "$config_report" = "on" ] || [ "$state" = "enable" ] ; then
        message_out "Report full configuration - reporting on"
        uci set awc_cloud.cloud.config_report=on
        report_full_config=true
 
      elif [ "$config_report" = "off" ] || [ "$state" = "disable" ] ; then    
        message_out "Report full configuration - reporting off"
        uci set awc_cloud.cloud.config_report=off
                  
      elif [ "$config_report" = "" ] || [ "$state" = "now" ] ; then   
        message_out "Report full configuration - reporting now"
        report_full_config=true    
      
      else 
        message_err "Report full configuration - unknown state: '$state'"
        return 1
          
      fi
       
      confirm_action=true
       
    elif json_get_var ssid "ssid" ; then 
       
       if ! uci get wireless.default_radio0 2>/dev/null; then
         message_err "Setting SSID not supported on this sytem"
         return 1
       fi
       
       if ! uci set wireless.default_radio0.ssid="$ssid" || 
          ! uci set wireless.default_radio1.ssid="$ssid" ; then
         message_err "Failed to set SSID to '$ssid'"
         return 1
       fi
     
       cycle_wifi=true
       confirm_action=true

    elif json_select lanip >/dev/null 2>/dev/null; then
    
       if json_get_var ipaddress "ip-address"; then
         
         json_get_var netmask "netmask" || true

       else 
         message_err "IP Address not specified"
         return 1
       fi
       
       message_out "Set LAN IP: '$ipaddress' netmask: '$netmask'"
    
       uci set network.lan.ipaddr="$ipaddress"
       if [ -n "$netmask" ] ; then
         uci set network.lan.netmask="$netmask"
       fi
       
       cycle_lan=true
       confirm_action=true
            
    elif json_select ssh >/dev/null 2>/dev/null; then
    
      json_get_var config_state     "enabled" >/dev/null || true
      json_get_var config_interface "interface" >/dev/null || true
      json_get_var config_port      "port" >/dev/null || true
      
      if [ "$config_state" != "on" ] && [ "$config_state" != "off" ] ; then
        message_err "Unsupported SSH state specified: '$config_state'"
        return 1
      fi
      
      if [ "$config_interface" != "lan" ] && [ "$config_interface" != "wwan" ] ; then
        message_err "Unsupported SSH interface specified: '$config_interface'"
        return 1
      fi
      
      if [ -z "$config_port" ] ; then
        config_port=22
      fi
      
      message_out "Set SSH state: $config_interface $config_state"
      
      instance=0
      found=false
      
      while true; do
        ssh_port=$(uci get dropbear.@dropbear[$instance].Port 2>/dev/null || true)
        
        # End of list
        if [ -z "$ssh_port" ] ; then break; fi
        
        ssh_interface=$(uci get dropbear.@dropbear[$instance].Interface 2>/dev/null || true)
        # For LAN, empty or "lan"
        if [ -z "$ssh_interface" ] ; then ssh_interface=lan; fi
        
        if [ "$config_interface" = "$ssh_interface" ] ; then
          if [ "$config_state" = "off" ] ; then
            uci set dropbear.@dropbear[$instance]=
          else   
            found=true
          fi 
          break
        fi
        
        instance=$((instance + 1))
      done
      
      # For add, if not found, then add
      if [ "$config_state" = "on" ] && ! $found ; then
        uci add dropbear dropbear >/dev/null
        uci set dropbear.@dropbear[$instance].Interface="$config_interface"
        uci set dropbear.@dropbear[$instance].Port="$config_port"
        uci set dropbear.@dropbear[$instance].PasswordAuth=on
        found=true
      fi

      ssh_restart=true
      confirm_action=true
    
    elif json_select "vpn_enable" >/dev/null 2>/dev/null; then

      # Parse the VPN Enable request.
      message_out "Enabling WireGuard VPN"
      json_get_var server_pub_ip server_pub_ip
      json_get_var peer_server_ip peer_server_ip
      json_get_var peer_public_key peer_public_key
      json_get_var server_priv_ip server_priv_ip
      json_get_var peer_client_ip peer_client_ip
      json_get_var listen_port listen_port
      json_get_var allowed_ip allowed_ip
      echo $server_pub_ip
      echo $peer_server_ip
      echo $peer_public_key
      echo $server_priv_ip
      echo $peer_client_ip
      echo $listen_port
      echo $allowed_ip
    
      # Enable the local VPN.
      ip link add wg0 type wireguard
      ip addr add $server_priv_ip dev wg0
      wg set wg0 listen-port $listen_port private-key /sbin/private peer $peer_public_key allowed-ips $allowed_ip endpoint $server_pub_ip:$listen_port persistent-keepalive 25
      ip link set wg0 up
      ip address add $peer_client_ip peer $peer_server_ip dev wg0

    else
      messasge_err "Unsupported action"
      return 1
      
    fi
    
  else  
    message_err "Parsing poll response failed: $data"
    return 1
  fi

  action_sequence=$((action_sequence + 1))
  commit=true
  return 0
}


perform_software_update() {

  update_action_sequence=${action_sequence}

  if [ -z "${update_action_sequence}" ] ; then
    update_action_sequence=$(uci get awc_cloud.cloud.action_sequence 2>/dev/null || true)
  fi

  if [ -z "${update_action_sequence}" ] ; then
    update_action_sequence=0
  fi
  
  pending=$((update_action_sequence + 1))

  if $noaction; then
    message_out "No action specified. Would set: "
    message_out "  uci set awc_cloud.cloud.action_sequence_pending=$pending"

  else
    # Record that we need to increase any pending action upon succesful upgrade
    uci set awc_cloud.cloud.action_sequence_pending=$pending
    uci commit

    if /sbin/update-check.sh -e; then
      : # Won't return if successful
    else
      # Upgrade failure, roll back any pending action
      message_err "Upgrade script failed"
      uci delete awc_cloud.cloud.action_sequence_pending 2>/dev/null || true
      uci commit
        #exit 1
    fi
  fi
}


perform_config_update() {
  message_out "Applying configuration updates"

  if /sbin/jsontouci.sh -n "$config_updates"; then
    action_sequence=$((action_sequence + 1))

    uci set awc_cloud.cloud.action_sequence=${action_sequence}
    uci commit

    rm -f "${config_updates}"

    # Force immediate poll to confirm the action
    message_out "Immediately poll cloud with new configuration"
    return 0
  fi

  return 1
}
: << 'COMMENT'
# Make sure that additional WiFi configuration values are defined
countrycode0=$(uci get wireless.radio0.country 2>/dev/null || true)
if [ "$countrycode0" == "" ] ; then
  uci set wireless.radio0.country=US
fi

countrycode1=$(uci get wireless.radio1.country 2>/dev/null || true)
if [ "$countrycode1" == "" ] ; then
  uci set wireless.radio1.country=US
fi

isolate0=$(uci get wireless.default_radio0.isolate 2>/dev/null || true)
if [ "$isolate0" == "" ] ; then
  uci set wireless.default_radio0.isolate=1
fi

isolate1=$(uci get wireless.default_radio1.isolate 2>/dev/null || true)
if [ "$isolate1" == "" ] ; then
  uci set wireless.default_radio1.isolate=1
fi

server0=$(uci get wireless.default_radio0.server 2>/dev/null || true)
if [ "$server0" == "" ] ; then
  uci set wireless.default_radio0.server=192.168.113.200
fi

server1=$(uci get wireless.default_radio1.server 2>/dev/null || true)
if [ "$server1" == "" ] ; then
  uci set wireless.default_radio1.server=192.168.113.200
fi

port0=$(uci get wireless.default_radio0.port 2>/dev/null || true)
if [ "$port0" == "" ] ; then
  uci set wireless.default_radio0.port=1812
fi

port1=$(uci get wireless.default_radio1.port 2>/dev/null || true)
if [ "$port1" == "" ] ; then
  uci set wireless.default_radio1.port=1812
fi

auth_secret0=$(uci get wireless.default_radio0.auth_secret 2>/dev/null || true)
if [ "$auth_secret0" == "" ] ; then
  uci set wireless.default_radio0.auth_secret=SecretForAP1
fi

auth_secret1=$(uci get wireless.default_radio1.auth_secret 2>/dev/null || true)
if [ "$auth_secret1" == "" ] ; then
  uci set wireless.default_radio1.auth_secret=SecretForAP1
fi

uci commit
COMMENT

# Make sure that the file /etc/sysupgrade.conf is up to date.
# If this Mega Fi originally had a version before 2.4.41 loaded then the 
# file /etc/sysupgrade.conf will be incorrect and a software update will never overwrite it.
# It is incorrect if it does not have the line "/etc/config/awc".
if ! cat /etc/sysupgrade.conf | grep -q "/etc/config/awc"; then
  # echo "awc not found"
  chmod 666 /etc/sysupgrade.conf
  echo "## This file contains files and directories that should" > /etc/sysupgrade.conf
  echo "## be preserved during an upgrade." >> /etc/sysupgrade.conf
  echo " " >> /etc/sysupgrade.conf
  echo "/etc/awc/" >> /etc/sysupgrade.conf
  echo "/etc/config/awc" >> /etc/sysupgrade.conf
  echo "/etc/config/awc_cloud" >> /etc/sysupgrade.conf
  echo "/etc/config/awc_gpsd" >> /etc/sysupgrade.conf
  echo "/etc/config/dropbear" >> /etc/sysupgrade.conf
  echo "/etc/config/fstab" >> /etc/sysupgrade.conf
  echo "/etc/config/network" >> /etc/sysupgrade.conf
  echo "/etc/config/system" >> /etc/sysupgrade.conf
  echo "/etc/config/ucitrack" >> /etc/sysupgrade.conf
  echo "/etc/config/wireless" >> /etc/sysupgrade.conf
  echo "/etc/config/dhcp" >> /etc/sysupgrade.conf
  echo "/etc/config/firewall" >> /etc/sysupgrade.conf
  echo "/etc/config/luci" >> /etc/sysupgrade.conf
  echo "/etc/config/rpcd" >> /etc/sysupgrade.conf
  echo "/etc/config/ubootenv" >> /etc/sysupgrade.conf
  echo "/etc/config/uhttpd" >> /etc/sysupgrade.conf
  echo "/etc/passwd" >> /etc/sysupgrade.conf
  echo "/etc/shadow" >> /etc/sysupgrade.conf
  echo "/etc/dropbear/dropbear_rsa_host_key" >> /etc/sysupgrade.conf
  echo "/etc/uhttpd.key" >> /etc/sysupgrade.conf
  echo "/etc/uhttpd.crt" >> /etc/sysupgrade.conf
  echo "/etc/openvpn" >> /etc/sysupgrade.conf
  echo "/etc/nftables.d" >> /etc/sysupgrade.conf
  chmod 400 /etc/sysupgrade.conf
fi

confirm_action=false
#nodelay=true
verbose=false
noaction=false
override_poll=
do_factory_reset=false
cycle_lan=false
cycle_wifi=false
ssh_restart=false
modem_report_period=0
modem_report_last=$(date +%s)

report_full_config=false

# Only for the very first run and loop of this script
# This avoids the delay when the script is reloaded due to config changes
first_time=true

# Until we have a successful poll, then poll at least every 10 minutes. 
first_success=false


while [ -n "$1" ] ; do

  flag=$1

#  if [ "$flag" = '-d' ] ; then
#    nodelay=true

  if [ "$flag" = '-v' ] ; then
    verbose=true

  elif [ "$flag" = "-n" ] ; then
    noaction=true

  elif [ "$flag" = "-p" ] ; then
    override_poll=$2
    shift

  else
    echo "Unknown flag: $flag" 1>&2
    exit 1
  fi

  shift
done




fetch_output=/tmp/nextivity/fetch_out
fetch_headers=/tmp/nextivity/fetch_err
software_updates=/etc/awc/software_updates
config_updates=/etc/awc/config_updates

json_status=/tmp/nextivity/modemstatus.json
json_status_cloud=/tmp/nextivity/modemstatus.cloud.json
first_check=/tmp/nextivity/first_check
partial=/tmp/nextivity/partial.json 


#if [ "$(uci get awc.main.bridge_mode)" = "on" ] ; then
#  bridge_mode=true
#else
bridge_mode=false
#fi

set_cloud_status "Startup"


if [ -e "${first_check}" ] ; then
  nodelay=true
else
  touch ${first_check}
fi

# Delay before we poll for the first time
delay=$(uci get awc_cloud.cloud.delay 2>/dev/null || true)

if [ -z "$delay" ] ; then
  delay=100
fi


config_modem_report=$(uci get awc_cloud.cloud.modem_report 2>/dev/null || true)


poll=$(get_poll)

last_message=

uuid_error=false
uuid_error_count=0


aw12_software="unknown"
aw12_imei="unknown"
awi2_iccid="unknown"


# If automatic updates are off, but we have an update file present, then
# we don't want to do any polling.


while true; do
  wakeup=false


  if $first_time; then
    if ! $nodelay; then
      message_out "Waiting $delay seconds, and then polling every $poll seconds"

      if ! sleep_wait $delay; then
        wakeup=true
        #message_out "Woken up to perform action"
      fi
    fi

    first_time=false

  else
    # Might have changed
    poll=$(get_poll)
    
    message="Waiting $poll seconds before next cloud poll"
    
    if [ -n "$last_message" ] ; then
      message="$last_message - $message"
      last_message=
    fi
    
    # Reduce repeated error reporting on UUID errors. 
    if [ "$uuid_error" ] ; then
      if [ "$uuid_error_count" = 0 ] ; then
        message_out "$message"
      fi
      
      uuid_error_count=$((uuid_error_count + 1))
      if [ "$uuid_error_count" = 10 ] ; then 
        uuid_error_count=0
      fi

    else 
      uuid_error_count=0
    
      message_out "$message"
    fi
    
    if ! sleep_wait "$poll"; then
      wakeup=true
      #message_out "Woken up to perform action"
    fi
  fi
  
  
  host=$(uci get awc_cloud.cloud.poll_host 2>/dev/null || true)

  if [ -z "$host" ] ; then
    message_err "Cloud host not configured"
    continue
  fi

  # This is a work around for Passthrough mode IP NAT
  if $bridge_mode; then
    if ! /sbin/bridge-rules.sh -host ei.awcone.com 2>/dev/null; then
      message_out "Bridge setup not ready"
      continue
    fi
  fi

  count=0
  ready=false
  # Wait for json and route

  while true; do
    if [ -e $json_status ] ; then
      if $bridge_mode; then
        ready=true
        break
        
      else 
        # Check ModemManager status
        #if modem_find; then 
          #if modem_connected; then
            ready=true
            break
          #fi
        #fi
        $verbose && echo "Waiting on modem connection"
      fi
    fi

    if ! sleep_wait 10; then
      # Woken up
      break
    fi
    
    count=$((count + 1))
    if [ "$count" = 10 ] ; then
      message_err "No Connection for Cloud Status"
      count=0
      #break; 
    fi
  done

  if ! $ready; then
    message_err "JSON file not found or no connection"
    continue
  fi

  json_init
  json_load_file $json_status

  if json_is_a status array; then
    json_select status
    idx=1

    while json_is_a ${idx} object; do
      json_select $idx >/dev/null || break
      json_get_var name name || true
      json_get_var value value || true

	  idx=$((idx + 1))

      #echo $name $value

      if [ -n "$value" ] ; then
        case $name in

          IMEI_s)
            aw12_imei=$value
            ;;

          ModemSwVersion_s)
            aw12_software=$value
            ;;

          ICCID_s)
            aw12_iccid=$value
            ;;
            
        esac
      fi

      json_select ..
    done
  fi

  # Send VPN public key and/or create as if not available
  vpn_pub_key=$(wg pubkey < /sbin/private 2>/dev/null || true)

  if [ -z "$vpn_pub_key" ] ; then
    umask 077
    wg genkey > /sbin/private
    vpn_pub_key=$(wg pubkey < /sbin/private)
    echo "$vpn_pub_key"
    else
  
    vpn_pub_key=$(wg pubkey < /sbin/private)
     
   fi

  json_init
  json_load_file $json_status
  if json_is_a status array; then
    json_select status
    json_add_object
    json_add_string 'name' 'vpn_pub_key'
    json_add_string 'value' "$vpn_pub_key"
    json_add_string 'lastChange' "1700000000"
    json_add_string 'lastUpdate' "1700000000"
    json_close_object
    json_select ..
  fi
  json_dump > $json_status_cloud

  # Don't poll until we have both IMEI and ICCID
  if [ "$aw12_imei" = "unknown" ] || [ "$aw12_iccid" = "unknown" ] ; then
    message_err "IMEI and ICCID not yet known - waiting"
    continue
  fi

  # Serial Number
  serialNumber=$(cat /tmp/nextivity/serialNumber 2>/dev/null || true)

  if [ -z "$serialNumber" ] || [ "$serialNumber" = "Unknown" ] ; then
    messasge_err "Serial number not known - not polling cloud"
    continue
  fi

  # MAC
  #mac=$(ifconfig eth0 | grep HWaddr | sed -e 's#.*HWaddr ##' -e 's# ##g')

  macpick=$(ifconfig ra0 | grep HWaddr | sed -e 's#.*HWaddr ##' -e 's# ##g')

  echo "$macpick"

  if [ -z "$macpick" ] ; then
    mac=$(ifconfig eth0 | grep HWaddr | sed -e 's#.*HWaddr ##' -e 's# ##g')
  else
    mac="$macpick"
  fi

  echo "$mac"


  if [ -z "$mac" ] ; then
    message_err "Cannot determine MAC address"
    continue
  fi

  macstrip=$(echo "$mac" | sed 's#:##g')
    
  debug "Poll time is $poll"

  confirm_action=false

  # Might not be set
  uuid=$(uci get awc_uuid.uuid_params.uuid 2>/dev/null || true)
  # Pre-version 2.4.41 way of storing UUID.
  # If this is defined then the UUID is invalid and this must be deleted
  clouduuid=$(uci get awc_cloud.cloud.uuid 2>/dev/null || true)
  if [ "$clouduuid" != ""  ] ; then
    uci delete awc_cloud.cloud.uuid
    uci delete awc_uuid.uuid_params.uuid
    uci commit
    uuid=""
  fi

    # Software version
  software_version=$(get_software_version)
  
  # Action Sequence
  action_sequence_pending=$(uci get awc_cloud.cloud.action_sequence_pending 2>/dev/null || true)
  auto_firmware=$(uci get awc_cloud.cloud.auto_firmware 2>/dev/null || true)
  auto_config=$(uci get awc_cloud.cloud.auto_config 2>/dev/null || true)

  update_software_version=$(uci get awc_cloud.cloud.expected_version 2>/dev/null || true)
  
  if [ -n "${action_sequence_pending}" ] ; then
    # In this case, we are confirming a value after an upgrade
    
    if [ "${update_software_version}" != "${software_version}" ]; then
      message_err "Mismatch on upgrade ($update_software_version) vs expected running version ($software_version)"
      
      # Reset
      uci delete awc_cloud.cloud.action_sequence_pending 2>/dev/null || true
      uci delete awc_cloud.cloud.expected_version 2>/dev/null || true
      uci commit
      # Fall through to confirm action
    fi 
  else
    # Left over 
    if [ -n "${update_software_version}" ] ; then
      uci delete awc_cloud.cloud.expected_version 2>/dev/null || true
      uci commit
    fi
  fi
  
  if [ -z "${action_sequence_pending}" ] && [ -e ${software_updates} ] ; then
    if [ "${auto_firmware}" != "on" ] && [ "$wakeup_reason" != "upgrade" ] ; then 
      # There is a pending upgrade, but auto update is off. In this case, do nothing and continue to wait
      message_out "Pending firmware upgrade, but auto update is off - waiting"
      set_cloud_status "Waiting on manual firmware update"
      continue
      
    else
      # There is a pending upgrade to be done - either normal polling, or manually triggered
      perform_software_update
      # If success, doesn't return
      continue

    fi

  elif [ -e ${config_updates} ] ; then
    if [ "${auto_config}" != "on" ] && [ "$wakeup_reason" != "config" ] ; then
      # Pending upgrades, wait
       message_out "Pending config upgrades, but auto update is off - waiting"
       continue

    else
      if perform_config_update; then
        message_out "Configuration updates applied"
        confirm_action=true
        # Continue on here to confirm action
        
      else
        message_err "Error applying configuration updates"
        confirm_action=false
        continue
      fi
    fi
  fi  
  
  if [ -n "${action_sequence_pending}" ] ; then
    action_sequence=${action_sequence_pending}
    
    message_out "Confirming upgrade to cloud with sequence ${action_sequence} to version ${update_software_version}"
    
  else 
    action_sequence=$(uci get awc_cloud.cloud.action_sequence 2>/dev/null || true)

    if [ -z "$action_sequence" ] ; then
      message_out "No action sequence set - starting at 0"
      action_sequence=0
    fi
  fi



  query="MAC=${macstrip}&SN=${serialNumber}"

# Build this JSON message
#
# {
#   "ei_status":{
#      "mac":"34BA9A7B4C49",
#      "software.version":"1.0.1",
#      "pending.action.sequence":0,
#      "aw12.imei":"869710030002905",
#      "aw12.firmwareversion":"EM12AWPAR01A07M4G",
#      "aw12.sim.iccid":"89011004300029885457"
#   }
# }

  if [ -e $registration_data ] && json_load_file $registration_data; then 
    json_get_var reg_first_name "first_name" || true
    json_get_var reg_last_name  "last_name"  || true
    json_get_var reg_company    "company"    || true
    json_get_var reg_phone      "phone"      || true
    json_get_var reg_email      "email"      || true
  else
    reg_first_name=
  fi
  
  json_init
  #json_add_object "${boardShort}_status"
  json_add_object "ei_status"
  json_add_string "mac" "$macstrip"
  json_add_string "software_version" "$software_version"
  json_add_int    "pending_action_sequence" "$action_sequence"
  json_add_string "aw12_imei" "$aw12_imei"
  json_add_string "aw12_firmwareversion" "$aw12_software"
  json_add_string "aw12_sim_iccid" "$aw12_iccid"


# Send VPN public key and/or create as if not available
  #vpn_pub_key=$(wg pubkey < /sbin/private 2>/dev/null || true)
#
  #if [ -z "$vpn_pub_key" ] ; then
  #  umask 077
  #  wg genkey > /sbin/private
  #  vpn_pub_key=$(wg pubkey < /sbin/private)
  #  echo "$vpn_pub_key"
  #  else
  #
  #  vpn_pub_key=$(wg pubkey < /sbin/private)
  #   
  # fi
 #
  #json_add_string "vpn_pub_key" "$vpn_pub_key"
  #
  json_close_object

  
  # Possible User/EULA reigstration data

  if [ -n "$reg_first_name" ] ; then
    #json_add_object "${boardShort}_eula"
    json_add_object "ei_eula"
    json_add_string "first_name" "$reg_first_name"
    json_add_string "last_name"  "$reg_last_name"
    json_add_string "company"    "$reg_company"
    json_add_string "phone"      "$reg_phone"
    json_add_string "email"      "$reg_email"
    json_close_object 
  fi
    
  
  # If not UUID, then do initial query
  if [ -z "$uuid" ] ; then
    #message_out "Fetching initial UUID"
    result=$(fetch "$host" "$query" || true)

    if [ "$result" != "200" ] ; then
      #cat $fetch_output
      #cat $fetch_headers

      if [ "$result" = "403" ] ; then
        # Forbidden. UUID assigned in cloud, but lost locally
        message_err "Error fetching UUID - already assigned in cloud"
      else
        message_err "Failed to fetch UUID with code: $result"
      fi
            
      uuid_error=true
      set_cloud_status "UUID fetch failed"

      continue
    else
	  new_uuid=$(cat "$fetch_output" 2>/dev/null || true)

      if [ -z "$new_uuid" ] ; then
        message_err "Failed to parse UUID"
        set_cloud_status "UUID parse failed"
        uuid_error=true
        continue
      fi

      message_out "UUID fetched: $new_uuid"
      uuid_error=false
    fi

    # Set date of last succesful fetch
    set_cloud_status "Connected"
  fi


  commit=true

  if [ -z "${new_uuid}" ] ; then
     # Use existing
    commit=false

  elif [ -z "${uuid}" ] ; then
    # Wasn't set
    uuid=${new_uuid}

  elif [ "${uuid}" != "${new_uuid}" ] ; then
    # Has changed, will use new
    message_out "New UUID (${new_uuid}) varies from present (${uuid})"
  else
    # Same, do nothing
    commit=false
  fi


  # Now do regular query
  query="${query}&PW=${uuid}"

  
  # Send modem status if requested and time
  do_report=false
  if [ "$modem_report_period" = "now" ] ; then
    debug "Modem report now"
    do_report=true
    modem_report_period=0

  else
    if [ -n "$config_modem_report" ] && [ "$config_modem_report" != 0 ] ; then
      now=$(date +%s)
      modem_report_period=$config_modem_report
      seconds=$modem_report_period
      diff=$((now - modem_report_last))

      debug "Modem report diff: $diff seconds: $seconds"

      if [ "$diff" -ge "$seconds" ] ; then
        do_report=true
      fi
      modem_report_last=$now
    fi
  fi

  # Check if we need to report a full config due to file changes, and we haven't
  # already been told to do so
  if ! $report_full_config ; then
    reporting_config=$(uci get awc_cloud.cloud.config_report 2>/dev/null || true)
    if [ "${reporting_config}" = "on" ] ; then
      report_full_config=true
    fi
  fi

  if ! [ -e "${sha256sumSave}" ] && [ $report_full_config ] ; then
    newConfigSha256=$(generate_json_all_config)
    $verbose && echo "Checking config change"

    if [ -e "${sha256sumSave}" ] ; then
      oldConfigSha256=$(cat $sha256sumSave)
      if [ "$oldConfigSha256" = "$newConfigSha256" ] ; then
        $verbose && echo "Config hasn't changed"
        report_full_config=false

      else
        $verbose && echo "Config has changed - will report"
        report_full_config=true
        echo -n "${newConfigSha256}" > ${sha256sumSave}

      fi

    else
      $verbose && echo "No saved sha256 - will at startup"
      report_full_config=true
      echo -n "${newConfigSha256}" > ${sha256sumSave}
    fi
  fi
  
  
  if $report_full_config; then
    newConfigSha256=${generate_json_all_config}
    json_dump | sed "s#\}\$#,#" > $partial
    cat $all_config >> $partial
    echo -n "}" >> $partial
  
    json_data=$(cat $partial)
    report_full_config=false

  elif $do_report; then
    json_dump | sed "s#\}\$#,#" > $partial
    echo -n "\"aw12_status\":" >> $partial
    sed 's#{.*status": ##' < $json_status_cloud  >> $partial
    
    json_data=$(cat $partial)
    
  else  
    json_data=$(json_dump)
  fi
    
  if $verbose; then
    debug "json_data: $json_data"
  fi
  
  result=$(fetch "$host" "$query" "$json_data")

  #echo "Result: $result"

  if [ "{result:0:6}" = "Error: " ] ; then
    last_message="Failed to perform cloud query"
    set_cloud_status "Retrying"
    continue

  elif [ "$result" != "200" ] ; then
    last_message="Failed polling cloud with error code: $result"
    set_cloud_status "Retrying"
    continue

  else
    last_message="Cloud poll returned"
    
  fi
  
  # Now do factory reset if needed
  if $do_factory_reset; then
    if $noaction; then
      message_out "Factory reset now" 
      /sbin/factory-reset.sh
      return 0 
    else
      message_out "Would do factory reset now, exiting"
    fi
  fi
    

  if [ -n "${action_sequence_pending}" ] ; then
    # Now we can forget about any pending value, since that's been sent if it was set
    # Also remove the old upgrade file
    uci delete awc_cloud.cloud.action_sequence_pending 2>/dev/null || true
    uci delete awc_cloud.cloud.expected_version 2>/dev/null || true
    commit=true
    rm -f $software_updates
  fi

  # Check returned data, if any
  data=$(cat $fetch_output 2>/dev/null || true)

  if [ -n "$data" ] ; then
    #echo "Data: $data"
    if ! handle_response "$data"; then
      set_cloud_status "Cloud Error"
      continue
    fi
  fi

  # Clear registration data after success
  rm -f "$registration_data"

  # Set date of last succesful fetch
  set_cloud_status "Connected"
  first_success=true
  
  if $commit; then
    $verbose && echo "Setting UUID here to ${uuid}"
    uci set awc_uuid.uuid_params.uuid="${uuid}"
    # Whether this is increased depends upon any actions
    if [ -n "${action_sequence}" ] ; then
      uci set awc_cloud.cloud.action_sequence=${action_sequence}
    fi
    uci commit
  fi

  
  if $cycle_lan; then
    ifdown lan || true
    sleep 1
    ifup lan || true
    cycle_lan=false
  fi
  
  if $cycle_wifi; then
    wifi reload 2>/dev/null || true
    cycle_wifi=false
  fi
  
  if $ssh_restart; then
    /etc/init.d/dropbear restart >/dev/null 2>/dev/null || true
    ssh_restart=false
  fi
  
 # if $report_full_config; then
 #   json=$(cat $all_config)
      
 #   debug "Full config report now"
 #   fetch $host $query $json
 #   report_full_config=false
 # fi
  
  # Now run updates, if any.  If we got this far, then
  # auto firmware update is enabled
  if [ -e ${software_updates} ] ; then
     perform_software_update
     # If success, doesn't return
  fi

  # If config updates are applied, then the poll is immediately done again (assuming configured to do so still)
  # to do the confirmation, otherwise wait.

  if [ -e ${config_updates} ] ; then
    if [ "${auto_config}" = "on" ] ; then
      if perform_config_update; then
        message_out "Configuration updates applied"
        confirm_action=true
      else
        message_err "Error applying configuration updates"
        confirm_action=false
      fi
    else
      message_out "Pending configuration updates, but auto update is off - waiting"
    fi
  fi

done
