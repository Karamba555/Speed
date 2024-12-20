#!/bin/sh -e

# Kill the sleep command in the cloud poll to force the upgrade or
# config to happen immediately. This is to faciliate manual
# handling of these actions

attempts=0

wakeup_reason=$1

if [ -z "$wakeup_reason" ] ; then
  wakeup_reason="poll"
fi

reason_file=/tmp/nextivity/wakeup-reason


echo -n "$wakeup_reason" > $reason_file

while true; do
  if [ "$attempts" == "3" ] ; then
    break
  fi
 
  pid=$(cat /tmp/nextivity/cloud-sleep.pid 2</dev/null || true)

  if [ -n "$pid" ] ; then
    if kill $pid 2>/dev/null; then
      exit 0
    fi
  fi

  # Try again
  attempts=$(($attempts + 1))
  sleep 5
done

# Fail, restart the script
logger "Cloud Wakeup failed; restarting script"
rm -f $reason_file
/etc/init.d/cloud-check restart
