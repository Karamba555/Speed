#!/bin/sh -e

TMP_DIR=/tmp/nextivity
DNS_CONF="$TMP_DIR/dnsmasq_pthrough.conf"

while true; do
  bridge_mode=$(uci get awc.main.bridge_mode 2>/dev/null || true)
  if [ "$bridge_mode" == "on" ]; then
    if [ -f $DNS_CONF ]; then
        saved_mac_addr=$(cat $TMP_DIR/lan_macaddr)
        cur_mac_addr=$(ip neigh | grep -m1 REACHABLE | awk {'print $5'} | tr '[a-z]' '[A-Z]' 2>/dev/null || true)
        echo "$saved_mac_addr"
        echo "$cur_mac_addr"
        ip=$(head -1 /tmp/nextivity/dnsmasq_pthrough.conf | awk -F , {'print $2'} 2>/dev/null || true)
      	conn_cnt=$(arp -a | grep $ip | grep ether 2>/dev/null || true)
	      if [ "$conn_cnt" != "" ]; then
	          #logger -t update_mac_addr "conn_cnt: $conn_cnt"
	          sleep 5
            continue;
	      fi
        # if another device was connected with another mac-address
        if [ ! -z "$cur_mac_addr" -a "$cur_mac_addr" != "$saved_mac_addr" ] ; then
            logger -t update_mac_addr "new macaddr: $cur_mac_addr"
            uci set network.wwan.mac=$cur_mac_addr
            echo $cur_mac_addr > $TMP_DIR/lan_macaddr
            echo $cur_mac_addr > /sys/class/net/wwan0/qmi/bridge_mac
            # read dhcp_host from file
            dhcp_host_ip=$(head -1 $DNS_CONF | awk -F , {'print $2'} 2>/dev/null || true)
            # check if there is host mac in the file
            cnt=$(wc -l $DNS_CONF | awk '{print $1}' 2>/dev/null || true)
            if [ "$cnt" == "2" ]; then
                # delete prev dhcp_host mac from file
                sed '$d' $DNS_CONF > $TMP_DIR/temp && mv $TMP_DIR/temp $DNS_CONF
            fi
            # write new dhcp_host value to file
            echo "dhcp-host=$cur_mac_addr,$dhcp_host_ip,12h" >> "$DNS_CONF"
            # clean leases
            echo "" > /etc/awc/dhcp/dhcp.leases
	    echo "" > /etc/awc/dhcp/odhcpd
            # use new settings
            /etc/init.d/dnsmasq restart
            ethtool -r lan1 2>/dev/null || true
            wifi 2>/dev/null || true
            sleep 3
        fi
    fi
  fi
  sleep 5
done
