#!/bin/sh -e

# Conver the system configuration as reported by 'uci show' to JSON.  This is
# just a flat array. We could instead generate individual sections, which might 
# be more appropriate.
# 
# Also could add other system configuration such as public keys and passwords.

objectName="config"

option=$1

if [ "$option" = "-n" ] ; then
  objectName="$2"
  shift 2
fi

outer=true

option=$1
if [ "$option" = "-o" ] ; then
  outer=false
  shift
fi

# Optionally only this section
sectionOnly=$1

json_entry() {
  field=$1
  shift
  value=$@

  echo -n \"$field\":\"$value\"
}


firstSection=true
firstValue=true
matchDone=false

if $outer; then
  echo -n "{\"${objectName}\":{"
else
  echo -n "\"${objectName}\":{"
fi
  

uci show $sectionOnly | while read -r line; do
  field=$(echo $line | sed -e "s#=.*##")
  
  #echo "Line '$line'"
  # Strip quotes
  value=$(echo $line | sed -e "s#^.*=##" -e "s#^'##" -e "s#'\$##")
   
  #echo $line | sed -e "s#^'##" -e "s#'\$##" | read -d '=' field value
   
  if [ -n "$sectionOnly" ] ; then
    section=$(echo $field | cut -d . -f 1)
    #echo "section: $section"
  
    if [ "$section" != "$sectionOnly" ] ; then
      if $matchDone; then 
        break
      else
        continue
      fi
    fi
    matchDone=true
  fi 
  
  # Check for new section
  if [ -z "$(echo $line | cut -d . -f 3)" ] ; then
    
    # End previous section 
    if $firstSection; then
      firstSection=false
       
    else
      echo -n "},"
    fi
     
    echo -n "\"$field=$value\":{"
     
    firstValue=true
    continue
  fi 
  
  if $firstValue; then
    firstValue=false
  else
    echo -n ","
  fi
  
  #json_entry $field $value
  echo -n \"$field\":\"$value\"
  
done

#echo "FirstSection: $firstSection $lines" 1>&2

# End last section if there were any values
# TODO: This is not exported from the read loop
#if ! $firstSection; then
  echo -n "}"
#fi

if $outer; then
  echo -n "}}"
else 
  echo -n "}"
fi



