#!/bin/sh

tz=`nvram_get 2860 TZ`

if [ "$tz" = "UCT_-11" ]; then
        cp -f /usr/share/zoneinfo/Pacific/Samoa /etc/localtime
elif [ "$tz" = "UCT_-10" ]; then
        cp -f /usr/share/zoneinfo/US/Hawaii /etc/localtime
elif [ "$tz" = "NAS_-09" ]; then
        cp -f /usr/share/zoneinfo/US/Alaska /etc/localtime
elif [ "$tz" = "PST_-08" ]; then
        cp -f /usr/share/zoneinfo/Pacific /etc/localtime
elif [ "$tz" = "MST_-07" ]; then
        cp -f /usr/share/zoneinfo/US/Mountain /etc/localtime
elif [ "$tz" = "CST_-06" ]; then
        cp -f /usr/share/zoneinfo/US/Arizona /etc/localtime
elif [ "$tz" = "UCT_-06" ]; then
        cp -f /usr/share/zoneinfo/Mexico/General /etc/localtime
elif [ "$tz" = "UCT_-05" ]; then
        cp -f /usr/share/zoneinfo/America/Indiana /etc/localtime
elif [ "$tz" = "EST_-05" ]; then
        cp -f /usr/share/zoneinfo/America/New_York /etc/localtime
elif [ "$tz" = "AST_-04" ]; then
        cp -f /usr/share/zoneinfo/Brazil/West /etc/localtime
elif [ "$tz" = "UCT_-04" ]; then
        cp -f /usr/share/zoneinfo/Canda/Eastern /etc/localtime
elif [ "$tz" = "UCT_-03" ]; then
        cp -f /usr/share/zoneinfo/America/Guyana /etc/localtime
elif [ "$tz" = "EBS_-03" ]; then
        cp -f /usr/share/zoneinfo/Brazil/East /etc/localtime
elif [ "$tz" = "EUT_-01" ]; then
        cp -f /usr/share/zoneinfo/Atlantic/Azores /etc/localtime
elif [ "$tz" = "UCT_000" ]; then
        cp -f /usr/share/zoneinfo/UCT /etc/localtime
elif [ "$tz" = "GMT_000" ]; then
        cp -f /usr/share/zoneinfo/GMT0 /etc/localtime
elif [ "$tz" = "MET_001" ]; then
        cp -f /usr/share/zoneinfo/Europe/Paris /etc/localtime
elif [ "$tz" = "MEZ_001" ]; then
        cp -f /usr/share/zoneinfo/Europe/Paris /etc/localtime
elif [ "$tz" = "UCT_001" ]; then
        cp -f /usr/share/zoneinfo/Europe/Paris /etc/localtime
elif [ "$tz" = "EET_002" ]; then
        cp -f /usr/share/zoneinfo/Israel /etc/localtime
elif [ "$tz" = "SAS_002" ]; then
        cp -f /usr/share/zoneinfo/Israel /etc/localtime
elif [ "$tz" = "IST_003" ]; then
        cp -f /usr/share/zoneinfo/Asia/Kuwait /etc/localtime
elif [ "$tz" = "MSK_003" ]; then
        cp -f /usr/share/zoneinfo/Asia/Kuwait /etc/localtime
elif [ "$tz" = "UCT_005" ]; then
        cp -f /usr/share/zoneinfo/Poland /etc/localtime
elif [ "$tz" = "UCT_05:30" ]; then
        cp -f /usr/share/zoneinfo/Poland /etc/localtime
elif [ "$tz" = "UCT_007" ]; then
        cp -f /usr/share/zoneinfo/Poland /etc/localtime
elif [ "$tz" = "CST_008" ]; then
        cp -f /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
elif [ "$tz" = "CCT_008" ]; then
        cp -f /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
elif [ "$tz" = "SST_008" ]; then
        cp -f /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
elif [ "$tz" = "AWS_008" ]; then
        cp -f /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
elif [ "$tz" = "JST_009" ]; then
        cp -f /usr/share/zoneinfo/Japan /etc/localtime
elif [ "$tz" = "KST_009" ]; then
        cp -f /usr/share/zoneinfo/Japan /etc/localtime
elif [ "$tz" = "UCT_010" ]; then
        cp -f /usr/share/zoneinfo/Pacific/Guam /etc/localtime
elif [ "$tz" = "AES_010" ]; then
        cp -f /usr/share/zoneinfo/Pacific/Guam /etc/localtime
elif [ "$tz" = "UCT_011" ]; then
        cp -f /usr/share/zoneinfo/Asia/Magadan /etc/localtime
elif [ "$tz" = "UCT_012" ]; then
        cp -f /usr/share/zoneinfo/Pacific/Fiji /etc/localtime
elif [ "$tz" = "NZS_012" ]; then
        cp -f /usr/share/zoneinfo/Pacific/Fiji /etc/localtime
else
        # by default, use +8 timezone , will change based on NV later
        cp -f /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
fi

