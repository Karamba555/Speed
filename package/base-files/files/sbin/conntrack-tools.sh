while read line
        do
        conntrack -D -s $line  1>/dev/null 2>&1
        conntrack -D -s $line  1>/dev/null 2>&1
        conntrack -D -s $line  1>/dev/null 2>&1
        conntrack -D -s $line  1>/dev/null 2>&1

done < /tmp/UserIpaddress
        conntrack -D -d 8.8.8.8 1>/dev/null 2>&1
        conntrack -D -d 8.8.8.8 1>/dev/null 2>&1
        conntrack -D -d 8.8.8.8 1>/dev/null 2>&1
        conntrack -D -d 8.8.8.8 1>/dev/null 2>&1
