#!/bin/sh

if [ ! -n "$1" ]; then
echo "$1"
echo "insufficient arguments!"
exit 0
fi

sleep 2;
LICENCE="$1"
offset="$2"
len=="$3"

#echo $LICENCE
#echo $offset
#echo $len

#cat $LICENCE
/sbin/mtd -p $offset -l $len write $LICENCE VoIP 2>/dev/null
cp $LICENCE /etc/aim.conf  -rf 2>/dev/null
echo "update Success" 
