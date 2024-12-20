#!/bin/sh -e

if [ "$1" = -f ] ; then
  force=true
else
  force=false
fi

message_out() {
  message="Password Reset: $@"
  echo $message
  logger "$message"
}

message_err() {
  message="Password Reset: $@"
  echo "$message" 1>&2
  logger "$message"
  exit 1
}

if ! $force && ! diff -q /rom/etc/shadow /etc/shadow >/dev/null; then
  # message_out "Already set"

  # For the debug purposes only!!!
  serial=$(cat /tmp/nextivity/serialNumber | xargs echo -n)
  password=$(password-megafi $serial)
  message_out "================PASSWORD=====================: $password"
  # end of debug

  exit 0
fi

serial=$(cat /tmp/nextivity/serialNumber | xargs echo -n)
password=$(password-megafi $serial)

if [ -z "$password" ] ; then
  message_err "No password generated"
fi

if echo -e "$password\r$password" | passwd root; then
  message_out "Password set"
else
  message_err "Failed to set"
fi


