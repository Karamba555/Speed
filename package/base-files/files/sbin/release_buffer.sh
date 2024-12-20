#!/bin/sh
#
#
# usage: release_buffer.sh
#
#. /sbin/global.sh
. /sbin/config.sh
#echo "this is for release memory"
mount -o atelremount,rw / 
aim stop 2>/dev/null
killall -9 aimd.s udhcpc easycwmpd dhcplisttool atelIDURM ltemanage host_atc syslogd dnsmasq wanfailovertool 2>/dev/null
rmmod nf_nat_pptp 2>/dev/null
rmmod nf_nat_tftp 2>/dev/null
rmmod nf_nat_sip 2>/dev/null
rmmod nf_nat_proto_gre 2>/dev/null
rmmod nf_nat_irc 2>/dev/null
rmmod nf_nat_h323 2>/dev/null
rmmod nf_nat_ftp 2>/dev/null
rmmod nf_nat_amanda 2>/dev/null
rmmod nf_conntrack_sip 2>/dev/null
rmmod sg 2>/dev/null
ifconfig ra0 down  2>/dev/null
ifconfig ra1 down  2>/dev/null
ifconfig rai0 down  2>/dev/null
ifconfig rai1 down  2>/dev/null
rmmod ahci   2>/dev/null
rmmod libahci  2>/dev/null
rmmod libata  2>/dev/null
rmmod pppoe  2>/dev/null
rmmod ppp_async  2>/dev/null
rmmod pppox  2>/dev/null
rmmod ppp_generic  2>/dev/null
rmmod esp4  2>/dev/null
rmmod ah4  2>/dev/null
rmmod af_key  2>/dev/null
rmmod xfrm_user  2>/dev/null
rmmod xfrm_ipcomp  2>/dev/null
rmmod xfrm_algo        2>/dev/null          
rmmod ipcomp   2>/dev/null
rmmod xfrm_ipcomp          2>/dev/null            
rmmod xfrm4_mode_beet       2>/dev/null   
rmmod xfrm4_mode_transport    2>/dev/null   
rmmod xfrm4_mode_tunnel     2>/dev/null    
rmmod xfrm4_tunnel   2>/dev/null    
rmmod xt_LOG        2>/dev/null          
rmmod xt_REDIRECT   2>/dev/null
rmmod xt_TCPMSS     2>/dev/null
rmmod xt_bpf       2>/dev/null
rmmod xt_comment    2>/dev/null
rmmod xt_esp         2>/dev/null
rmmod xt_limit    2>/dev/null
rmmod xt_mac   2>/dev/null
rmmod xt_mark    2>/dev/null
rmmod xt_multiport   2>/dev/null 
rmmod xt_nat   2>/dev/null
rmmod xt_policy   2>/dev/null
rmmod xt_state   2>/dev/null
rmmod xt_string   2>/dev/null
rmmod xt_tcpudp   2>/dev/null
rmmod xt_time   2>/dev/null
rmmod rlt_wifi 2>/dev/null
rmmod mt7603e 2>/dev/null
rmmod mt7628  2>/dev/null
if [ "$CONFIG_ATEL_PCB_PD80" = "y" -o "$CONFIG_ATEL_PCB_UR28" = "y" ]; then
	sleep 3
fi
rmmod mt76x2e 2>/dev/null
rmmod xt_FLOWOFFLOAD  2>/dev/null 
rmmod sd_mod  2>/dev/null 
rmmod xt_TCPMSS   2>/dev/null 
rmmod sha256_generic  2>/dev/null 
rmmod sha512_generic     2>/dev/null 
rmmod iptable_mangle  2>/dev/null 
rmmod  iptable_filter  2>/dev/null 
rmmod  ipt_ah  2>/dev/null 
rmmod  ipt_REJECT  2>/dev/null 
rmmod  ipt_MASQUERADE  2>/dev/null 
rmmod  ip_tables  2>/dev/null 
rmmod  scsi_mod 2>/dev/null 
rmmod  xt_time  2>/dev/null 
rmmod  xt_tcpudp  2>/dev/null 
rmmod  xt_conntrack 2>/dev/null 
rmmod  nf_flow_table_hw   2>/dev/null 
rmmod  nf_flow_table   2>/dev/null 
rmmod  x_tables  2>/dev/null
if [ "$CONFIG_ATEL_PCB_PD80" != "y" -o "$CONFIG_ATEL_PCB_UR28" = "y" ]; then
	ifconfig ra0 down  2>/dev/null
	ifconfig ra1 down  2>/dev/null
	ifconfig rai0 down  2>/dev/null
	ifconfig rai1 down  2>/dev/null
	rmmod mt7603e 2>/dev/null
	rmmod mt76x2e 2>/dev/null
fi
rmmod fxs ralink_pcm ralink_gdma  2>/dev/null 
cd /bin;rm -rf ated ntpclient dtca_atel config_generate sed_atel 2>/dev/null 
cd /usr/bin; rm -rf aim* regVoip client_thread host_atc  cfg_backup cfg_upgrade diag_transfer factory* fw_upgrade init_system  serial_tool *emoteUpgrade* lte_cpe_Upgrade_local ping_watchdog    miniupnpd iperf atelIDURM  miniupnpd curl  ip dnsmasq   wps_tool wanfailovertool wanfailover atelIDURM diagtransfer dhcplisttool ltemanage iperf3 htop 2>/dev/null 
cd /usr/bin; rm -rf arp-scan  usign    jsonfilter  fwtool config_encrypt iwinfo 2>/dev/null 
rm -rf /lib/modules 2>/dev/null
cd /lib/; rm -rf xtables libcurl.so* libfl*  libxtables* libip6tc.so* libip4tc.so* libmme* libebt* netifd   ipsec* libmount* *vpn* libnl-route* 2>/dev/null
cd /usr/lib/; rm -rf xtables libcurl.so* *dhcp* dnsmasq  libfl*  libgmp* libxtables* libip6tc.so* libip4tc.so* libmme* libebt*  ipsec* x *vpn*    2>/dev/null

rm -rf /etc_ro/ralink_pcm.ko /etc_ro/0B* /etc_ro/licence /etc_ro/easycwmp /etc_ro/Wireless  /etc_ro/lighttpd/www/wan /etc_ro/lighttpd/www/lte /etc_ro/lighttpd/www/firewall  /etc_ro/lighttpd/www/wireless /etc_ro/lighttpd/www/internet  2>/dev/null 
rm -rf /etc_ro/lighttpd/www/lang/zhcn /etc_ro/lighttpd/www/lang/spanish /etc_ro/lighttpd/www/lang/polish /etc_ro/lighttpd/www/lang/zhtw 2>/dev/null 
rm -rf /etc_ro/lighttpd/www/idu/graphics /etc_ro/lighttpd/www/usb /etc_ro/lighttpd/www/antenna /etc_ro/lighttpd/www/odumanage /etc_ro/lighttpd/www/vlan  2>/dev/null
rm -rf /etc_ro/lighttpd/www/idu/pic   /etc_ro/lighttpd/www/idu/pic_U270   /etc_ro/lighttpd/www/idu/pic_U270   2>/dev/null 
rm -rf /etc_ro/lighttpd/www/idu/bodyinfo_* /etc_ro/lighttpd/www/idu/bodylte_* 2>/dev/null
rm -rf /etc_ro/lighttpd/www/idu/bodysetting_wifi*  /etc_ro/lighttpd/www/idu/webbody_*  /etc_ro/lighttpd/www/idu/bodysms*  2>/dev/null
rm -rf /etc_ro/lighttpd/www/idu/style/*sms*  2>/dev/null
cd /etc_ro/lighttpd/www/adm/;rm -rf admin.shtml auth.shtml ddns.shtml diagnostic.shtml                                                                                             
rm -rf login_err.shtml login_err_polish.shtml management.shtml nvram.shtml pin.shtml settings.shtml statistic.shtml 
rm -rf status.shtml syslog.shtml system_command.shtml test.shtml test_mac.shtml wizard.shtml
rm -rf /tmp/atelLog /tmp/allsyslog /tmp/allmessage  /tmp/ipsec* 2>/dev/null
cd /etc_ro/lighttpd/www/cgi-bin;rm -rf ExportOpenvpn* firewall.cgi  wireless.cgi  tool.cgi adm.cgi upload_voip_licence.cgi upload_settings.cgi  management.cgi upload_eoc.cgi upload_bootloader.cgi  upload_all.cgi upload2.cgi sms.cgi lte.cgi internet.cgi 2>/dev/null 
rm /usr/share 2>/dev/null
cd /sbin;rm -rf ip-tiny   procd *vpn* uci kmodloader jffs2reset  3g* conntrack* reset*  wifi* dhcp*   swconfig logread  logd udhcpc* vo* s* 2>/dev/null
cd /usr/sbin;rm -rf ubi* wpad openvpn tcpdump  pppd  dnsmasq ripd inadyn miniupnpd minicom network_monitor bmon easycwmp xtables-multi ascii-xfr iwconfig iw quagga.init ipsec easycwmpd dmc dtca_atel 2>/dev/null
cd /etc/; rm -rf ipsec* udhcp* dropbear* strongswan* wireless*  iproute2 oui.txt edb* localtime 2>/dev/null 




echo 3 > /proc/sys/vm/drop_caches 
