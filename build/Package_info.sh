#!/bin/sh

. $1/package/base-files/files/sbin/config.sh
rm -rf Package_info.txt
touch Package_info.txt
if [ "$CONFIG_CUSTOMER_POLSAT" == "y" ]; then
echo "SW Path:;" >> Package_info.txt
fi

echo "Allow List:Disable;" >> Package_info.txt
echo "Allow SingleList:Disable;" >> Package_info.txt
echo "Latest Version:$2;" >> Package_info.txt
if [ "$CONFIG_MANUFACTURE_KT" == "y" ]; then
        echo "LTE Version:Disable;" >> Package_info.txt
fi
echo "Start Version:Disable;" >> Package_info.txt
echo "Start LTE Version:Disable;" >> Package_info.txt
echo "Check Interval:24;" >> Package_info.txt

if [ "$CONFIG_MANUFACTURE_MARVELL" == "y" ]; then
        echo "LTE Version Name:;" >> Package_info.txt
        echo "LTE AP Version:;" >> Package_info.txt
        echo "LTE CP Version:;" >> Package_info.txt
        echo "LTE SW Size:0;" >> Package_info.txt
fi
if [ "$CONFIG_CUSTOMER_ORANGE" == "y" ]; then
        echo "Force:Disable;" >> Package_info.txt
fi

cp Package_info.txt $1/image/

