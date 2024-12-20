#!/bin/sh

DEVICE=wwan0
WIF=wwan0
LIF=br-lan
IFCONFIG=/sbin/ifconfig
ROUTE=/sbin/route
IPTABLES=/usr/sbin/iptables
NFT=/usr/sbin/nft
FW4=/sbin/fw4
TMP_DIR=/tmp/nextivity
FWT="$TMP_DIR/pbridge_firewall.sh"
DNS_CONF="$TMP_DIR/dnsmasq_pthrough.conf"
mkdir -p "$TMP_DIR"


get_new_address() {
        p=`uci -q get network.lan.ipaddr`

        temp=`echo $p|awk -F '.' '{print $1"."$2"."$3}'`
        for i in `seq 245 254`; do
                addr=$temp.$i
                ping $addr -c1 -W1 2>/dev/null
                if [ "$?" -eq "1" ]; then
                        break
                else
                        continue
                fi
        done
        echo $addr > $TMP_DIR/lan_ip
}

iptohex() {
  ip=$1
  
  ip1=$(echo $ip | cut -d . -f 1)
  ip2=$(echo $ip | cut -d . -f 2)
  ip3=$(echo $ip | cut -d . -f 3)
  ip4=$(echo $ip | cut -d . -f 4)
  
  printf "%02x%02x%02x%02x" $ip1 $ip2 $ip3 $ip4
}

# delete previous hosts from the route table
while true; do
   host=$(route | grep UH | grep -m1 wwan0 | awk {'print $1'})
   if [ "$host" == "" ]; then
       break
   else
       route delete $host
   fi 
done

while true; do
   host=$(route | grep UH | grep -m1 br-lan | awk {'print $1'})
   if [ "$host" == "" ]; then
       break
   else
       logger "route delete $host"
       route delete $host
   fi 
done

WIP=$($IFCONFIG wwan0 | grep "inet addr" | awk {'print $2'} | awk -F ":" {'print $2'})
echo "WIP: $WIP"
logger -t passthrough.sh "WIP: $WIP"
WGW=$($ROUTE | grep wwan0 | grep default | awk {'print $2'})
echo "WGW: $WGW"
logger -t passthrough.sh "WGW: $WGW"
SUBNET=$($ROUTE | grep wwan0 | grep "*" | awk {'print $3'})
echo "SUBNET: $SUBNET"
logger -t passthrough.sh "SUBNET: $SUBNET"

if [ -z "$WIP" -o -z "$WGW" -o -z "$SUBNET"  ]; then
    return 1
fi

# changes the source IP address of packets coming from the SUBNET and going out through the wwan0 interface to WIP.
LIP=`uci get network.lan.ipaddr`
LNM=`uci get network.lan.netmask` 

echo "nft add table nat"
LNM=$(ip -f inet -o a s scope global wwan0 | grep -Po 'inet \S+' | sed 's#.*/\(.*\)#\1#')
echo "LNM: $LNM"
$NFT flush chain inet fw4 srcnat_wan 2>/dev/null || true
$NFT flush chain ip nat postrouting 2>/dev/null || true
$NFT add table nat
$NFT add chain nat postrouting { type nat hook postrouting priority 0 \; }
$NFT add rule nat postrouting ip saddr $LIP/24 oifname $WIF snat to $WIP

echo "ifconfig $LIF:0 down"
logger -t passthrough.sh "ifconfig $LIF:0 down"
$IFCONFIG $LIF:0 down

get_new_address
lan_ip=$(cat $TMP_DIR/lan_ip)
logger -t passthrough.sh "new_lan_ip: $lan_ip"
logger -t passthrough.sh "ifconfig $WIF $lan_ip up" 
$IFCONFIG $WIF $lan_ip up

#       replace default route to Gateway through WIF
echo $WGW
echo "route add -host $WGW dev $WIF"
logger -t passthrough.sh "route add -host $WGW dev $WIF"
$ROUTE add -host $WGW dev $WIF
echo "route add default gw $WGW dev $WIF"
logger -t passthrough.sh "route add default gw $WGW dev $WIF"
$ROUTE add default gw $WGW dev $WIF
#       add route to WAN IP through LAN iface
echo "route add -host $WIP dev $LIF"
logger -t passthrough.sh "route add -host $WIP dev $LIF"
$ROUTE add -host $WIP dev $LIF
# enable proxy_arp so can use WGW s gateway on LAN device
echo "1" >/proc/sys/net/ipv4/conf/$WIF/proxy_arp
echo "1" >/proc/sys/net/ipv4/conf/$LIF/proxy_arp

new_WGW=`echo $WIP | awk -F '.' '{print $1"."$2"."$3}'`

if [ "$WGW" != "$new_WGW.1" ] && [ "$WIP" != "$new_WGW.1" ]; then
        new_WGW="$new_WGW.1"
elif [ "$WGW" != "$new_WGW.2" ] && [ "$WIP" != "$new_WGW.2" ]; then
        new_WGW="$new_WGW.2"
else
        new_WGW="$new_WGW.3"
fi

echo "new_WGW=$new_WGW"

echo "ifconfig  $LIF:0 $new_WGW netmask 255.255.255.0"
logger -t passthrough.sh "ifconfig  $LIF:0 $new_WGW netmask 255.255.255.0"
$IFCONFIG  $LIF:0 $new_WGW netmask 255.255.255.0
echo "WIP: $WIP"
hex=$(iptohex $WIP)
echo $hex > /sys/class/net/wwan0/qmi/bridge_ipv4
#leasetime=`uci -q get network.wwan.leasetime`
echo "dhcp-range=lan,$WIP,$WIP,255.255.255.0,12h" > "$DNS_CONF"

macaddr=$(ip neigh | grep -m1 REACHABLE | awk {'print $5'} | tr '[a-z]' '[A-Z]')
logger -t passthrough.sh "macaddr: $macaddr"
uci set network.wwan.mac=$macaddr
echo $macaddr > $TMP_DIR/lan_macaddr
echo $macaddr > /sys/class/net/wwan0/qmi/bridge_mac

DMAC=`uci -q get network.wwan.mac`

if [ "$DMAC" ]; then
        #echo "dhcp-host=$DMAC,192.168.1.151,24h" >> "$DNS_CONF"
        echo "dhcp-host=$DMAC,$WIP,12h" >> "$DNS_CONF"
fi
/etc/init.d/dnsmasq restart
#/etc/init.d/firewall reload

ethtool -r lan1 2>/dev/null || true
wifi 2>/dev/null || true

echo "1" > /tmp/nextivity/passthrough_is_switched

sleep 1
