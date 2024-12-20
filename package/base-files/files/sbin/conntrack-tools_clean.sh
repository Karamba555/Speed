ip=`nvram_get 2860 lan_ipaddr`
addr=${ip%.*}
third=${addr##*.}

loop=5
while [ "$loop" -gt 0 ]
do
        echo "conntrack -D -s 192.168.$third.$loop"
        conntrack -D -s 192.168.$third.$loop 1>/dev/null 2>&1
        conntrack -D -s 192.168.$third.$loop 1>/dev/null 2>&1
        conntrack -D -s 192.168.$third.$loop 1>/dev/null 2>&1
        conntrack -D -s 192.168.$third.$loop 1>/dev/null 2>&1
        loop=`expr $loop - 1`
done
