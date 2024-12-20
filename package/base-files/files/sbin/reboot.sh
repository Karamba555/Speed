#!/bin/sh
. /sbin/config.sh
if [ "$CONFIG_USER_I2C_OPENWRT" = "y"  ]; then
gpio F LTE && sleep 5 && gpio N LTE &&  sleep 3 &&  reboot &
else
gpio F LTE && sleep 5 && reboot &
fi
