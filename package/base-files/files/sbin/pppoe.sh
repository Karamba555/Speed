#!/bin/sh

PPPOE_FILE=/etc/options.pppoe

if [ ! -n "$4" ]; then
  echo "insufficient arguments!"
  echo "Usage: $0 <user> <password> <eth_name> <opmode>"
  exit 0
fi

PPPOE_USER_NAME="$1"
PPPOE_PASSWORD="$2"
PPPOE_IF="$3"
PPPOE_OPMODE="$4"
PPPOE_IDLETIME="$5"

echo "noauth" > $PPPOE_FILE
echo "user '$PPPOE_USER_NAME'" >> $PPPOE_FILE
echo "password '$PPPOE_PASSWORD'" >> $PPPOE_FILE
#echo "hide-password" >> $PPPOE_FILE
echo "noipdefault" >> $PPPOE_FILE
echo "defaultroute" >> $PPPOE_FILE
#echo "nodetach" >> $PPPOE_FILE
echo "usepeerdns" >> $PPPOE_FILE
if [ $PPPOE_OPMODE == "KeepAlive" ]; then
	echo "persist" >> $PPPOE_FILE
elif [ $PPPOE_OPMODE == "OnDemand" ]; then
	PPPOE_IDLETIME=`expr $PPPOE_IDLETIME \* 60`
	echo "demand" >> $PPPOE_FILE
	echo "idle $PPPOE_IDLETIME" >> $PPPOE_FILE
fi
echo "mtu 1492" >> $PPPOE_FILE 
echo "mru 1492" >> $PPPOE_FILE 
echo "ipcp-accept-remote" >> $PPPOE_FILE 
echo "ipcp-accept-local" >> $PPPOE_FILE 
#echo "lcp-echo-failure 10" >> $PPPOE_FILE
#echo "lcp-echo-interval 30" >> $PPPOE_FILE
echo "ktune" >> $PPPOE_FILE
echo "default-asyncmap nopcomp noaccomp" >> $PPPOE_FILE
echo "novj nobsdcomp nodeflate" >> $PPPOE_FILE
echo "plugin /usr/lib/pppd/2.4.7/rp-pppoe.so $PPPOE_IF" >> $PPPOE_FILE
echo "ip-up-script /lib/netifd/ppp-up" >> $PPPOE_FILE
echo "ip-down-script /lib/netifd/ppp-down" >> $PPPOE_FILE
