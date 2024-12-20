#!/bin/sh

wwan="wwan0"
wan="wan"

ubus listen | while read -r line; do
    if echo "$line" | grep -q '"status":"wwan_up"'; then
        route=$(route | grep default | awk {'print $8'})                                                 
        if [ "$route" == "$wwan" ]; then                                                                 
            ip_addr=$(ifconfig $wan | grep inet  | awk {'print $2'} | cut -c6-20 2>/dev/null || true)    
            if [ "$ip_addr" != "" ]; then                                                                
                ip route del wwan                                                                        
                logger -t failover "udhcpc -q -f -n -i $wan"                                             
                udhcpc -q -f -n -i $wan                                                                  
                ip_addr=$(ifconfig $wan | grep inet  | awk {'print $2'} | cut -c6-20 2>/dev/null || true)
                if [ "$ip_addr" != "" ]; then                         
                    ip route add default via $ip_addr                 
                fi                                                    
            fi                                                        
        fi
    elif echo "$line" | grep -q '"status":"wan_down"'; then
        logger -t failover "udhcpc -q -f -n -i $wwan"
        udhcpc -q -f -n -i $wwan
    fi
done
