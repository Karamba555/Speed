#!/bin/sh


# Path to the file storing known clients
CLIENT_FILE="/tmp/nextivity/dhcp_client"

# DHCP request handling
handle_dhcp_request() {
    local client_mac="$1"
    #if [ "$action" = "add" ]; then
	conn_dev_mav=$(ip neigh show dev br-lan | grep -m1 REACHABLE | awk {'print $3'})
 	logger -t dhcp-script.sh "REACHABLE $conn_dev_mac"
    logger -t dhcp-script.sh "ADD: Client $client_mac"
}

# Main
if [ "$1" = "add" ] || [ "$1" = "del" ]; then
    bridge_mode=$(uci get awc.main.bridge_mode 2>/dev/null || true)
    if [ "$bridge_mode" == "on" ]; then
        handle_dhcp_request "$2"
    fi
fi
