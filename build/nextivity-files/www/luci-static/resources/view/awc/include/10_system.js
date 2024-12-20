'use strict';
'require baseclass';
'require fs';
'require rpc';
'require uci';
'require network';

var modemStatus;

var callSystemBoard = rpc.declare({
	object: 'system',
	method: 'board'
});

var callSystemInfo = rpc.declare({
	object: 'system',
	method: 'info'
});


function progressbar(value, max, byte) {
	var vn = parseInt(value) || 0,
	    mn = parseInt(max) || 100,
	    fv = byte ? String.format('%1024.2mB', value) : value,
	    fm = byte ? String.format('%1024.2mB', max) : max,
	    pc = Math.floor((100 / mn) * vn);

	return E('div', {
		'class': 'cbi-progressbar',
		'title': '%s / %s (%d%%)'.format(fv, fm, pc)
	}, E('div', { 'style': 'width:%.2f%%'.format(pc) }));
}


return baseclass.extend({
	title: _('Network Status'),
    id: 'system-status',

    loadStatus : function() {
      L.resolveDefault(fs.read('/tmp/nextivity/modemstatus.json')).
       then(L.bind(function (status) {
         modemStatus = JSON.parse(status);
         
       }, this))
       .catch(L.bind(function (err) {
          //console.log("Error during modemstatus.js loading", err);
          modemStatus = null;
          
       }, this)); 
    },

	load: function() {
		return Promise.all([
			L.resolveDefault(callSystemBoard(), {}),
			L.resolveDefault(callSystemInfo(), {}),
			fs.lines('/usr/lib/lua/luci/version.lua'),
            fs.read('/tmp/nextivity/serialNumber').catch(L.bind(function() { return ""})),
            L.resolveDefault(uci.load('network')),
            L.resolveDefault(uci.load('awc')),
            fs.lines('/etc/nextivity_build_info'),
            network.getNetworks(),
            L.resolveDefault(fs.read('/tmp/nextivity/cloud_state.json')).catch(function (err) { }),
            L.resolveDefault(fs.read('/tmp/nextivity/post_state.json')).catch(function (err) { }),              
            this.loadStatus(),
            L.resolveDefault(uci.load('network'))
        ]);
	},
    
    fieldsAppendStatus: function(table, fields) {
        for (var i = 0; i < fields.length; i += 2) {
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '40%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [ (fields[i + 1] != null) ? fields[i + 1] : '?' ])
			]));
		}
    },
    
    fieldsAppendMem: function(table, fields) {
        for (var i = 0; i < fields.length; i += 3) {
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '40%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [
					(fields[i + 1] != null) ? progressbar(fields[i + 1], fields[i + 2], true) : '?'
				])
			]));
		}
    },
    
	render: function(data) {
		var boardinfo    = data[0],
		    systeminfo   = data[1],
		    luciversion  = data[2],
            serialNumber = data[3],
            //build        = data[6],
            networks     = data[7];
	    var cloudStateJson  = data[8];
            var postStateJson   = data[9];

        var model =  "SHIELD MegaFi 2";

        var mem = L.isObject(systeminfo.memory) ? systeminfo.memory : {};

        if (!serialNumber || serialNumber == "") serialNumber = "Unknown";

        //var firmware = build[6] ? build[6].substr(19) : "Unknown"; // "Firmware Version: [Version]"

		luciversion = luciversion.filter(function(l) {
			return l.match(/^\s*(luciname|luciversion)\s*=/);
		}).map(function(l) {
			return l.replace(/^\s*\w+\s*=\s*['"]([^'"]+)['"].*$/, '$1');
		}).join(' ');

        if (boardinfo.model == "ATEL-AW12") {
            boardinfo.model = "Nextivity Ethernet Injector";
        }

        // The prototype IP suggested display also of an IPV6 address, but that's not 
        // common on the bridge IP
        var lanip = uci.get('network', 'lan', 'ipaddr');
        var rxbytes;
        var txbytes;
        
        //console.log("Networks: " + networks);
        for (var count = 0; count < networks.length; count++) {
			var ifc = networks[count];
         
            //console.log("network name: " + ifc.getName());
            if (ifc.getName() == "lan") {
                var dev = ifc.getL3Device() || ifc.getDevice();
                
                if (dev) {
                    rxbytes = '%.2mB (%d %s)'.format(dev.getRXBytes(), dev.getRXPackets(), _('Pkts.'));
                    txbytes = '%.2mB (%d %s)'.format(dev.getTXBytes(), dev.getTXPackets(), _('Pkts.'));
                }
                break;
            }
        }
   
        try {
            if (modemStatus) {
              modem = JSON.parse(modemStatus);
            }
        } catch (exception) {
            
        }
        
        var cloud_connection_state = "(Unknown)";
        
         
        try {
            if (cloudStateJson) {
                var cloudState = JSON.parse(cloudStateJson);

                if (cloudState && cloudState.state) {
                    cloud_connection_state = cloudState.state;
                
                    if (cloud_connection_state == "Connected") {
                        var d = new Date(cloudState.time * 1000);
                        var cloud_date = d.toLocaleString();
                    
                        cloud_connection_state = "Connected (" + cloud_date + ")";
                    }
                }
            }
            
        } catch (exception) {
            //console.log("no cloud state");
        }
        
        var post_state = "(Unknown)";
        
        try {
            if (postStateJson) {
                var postState = JSON.parse(postStateJson);
            
                if (postState && postState.state) {
                    post_state = postState.state;
         
                    if (post_state != "Disabled") {
                        var d = new Date(postState.time * 1000);
                        var post_date = d.toLocaleString();
                    
                        post_state += " (" + post_date + ")";
                    }
                }
            }
        } catch (exception) {
            //console.log("No post state");

        }

        var apn = uci.get('network', 'wwan', 'apn');
        var systemName;
        var netmode;
        var connection, percent;
        var fb, band, homenetwork, txpower, rxpowerC0, rxpowerC1, cid;
        var rsrq, rsrp, rssi, snr, phycid;
        var mcc = '', mnc = '';
        var lat, lon;
        var imei, iccid, phonenumber;
        var expiredNames = [];
        
        // TODO: Reuse table from modem status
        
        if (modemStatus && modemStatus.status) {
            for (var index in modemStatus.status) {
                var entry = modemStatus.status[index];
                var name       = entry.name;
                var value      = entry.value;
                var lastUpdate = entry.lastUpdate;

                // Process value first before doing the expired formatting
                if (lastUpdate == -1) {
                  expiredNames[name] = true;
                }

                if (name == "ScFb_i") {
                    fb = value;
                } else if (name == "MCBV_s") {
                    band = value;
                } else if (name == "HnName_s") {
                    homenetwork = value;
                } else if (name == "HnMcc_i") {
                    mcc = value;
                } else if (name == "HnMnc_i") {
                    mnc = value;
                } else if (name == "Connection_s") {
                    connection = value;
                } else if (name == "SignalPercent_s") {
                    percent = value;
                } else if (name == "TX_Power") {
                    txpower = value;
                } else if (name == "ScId_i") {
                    cid = value;
                } else if (name == "ScRsrq_i") { 
                    rsrq = value;
                } else if (name == "ScRsrp_i") { 
                    rsrp = value;
                } else if (name == "ScRssi_i") { 
                    rssi = value;
                } else if (name == "ScSinr_f") {     
                     value = parseFloat(value).toFixed(1); 
                    snr = value;
                } else if (name == "Lat_f") {
                    lat = value;
                } else if (name == "Lon_f") {
                    lon = value;
                } else if (name == "IMEI_s") {
                    imei = value;
                } else if (name == "PhoneNumber_s") {
                    phonenumber = value;
                } else if (name == "ICCID_s") {
                    iccid = value;   
                } else if (name == "ScPId_i") {
                    phycid = value;
                } else if (name == "SystemName_s") {
                    systemName = value;
                } else if (name == "NetMode_s") {
                    netmode = value;
                }
            }
        }
        
        if (band == undefined) {
            band = "(Unknown)";
        } else if (band == "0x260") {
            band = "Any";
        }

        if (txpower == undefined) {
            txpower = "(Updating)"; // Consistent with modem status page
        }

        var homenetwork_label;

        if (!mcc || !mnc) {
            homenetwork_label = _("Home Network");
            
        } else if (homenetwork == (mcc + " " + mnc)) {
            // This case should no longer happen any more due to the additional processing we now
            // do, but previously the Home Network name was constructed by libqmi to be this string.
            // We now do additional calls to determine the actual name.
            homenetwork_label = _("Home Network") + " (MCC MNC)";
            
        } else {
            // Otherwise, put those values after anyway
            
            homenetwork_label = _("Home Network") + " (MCC,MNC)";
            homenetwork = homenetwork + " (" + mcc + "," + mnc + ")";
            
        }

        var location;
        
        if (lat == undefined || lon == undefined) {
            location = "(Unknown)";
        } else {
            location = parseFloat(lat).toFixed(6) + "," + parseFloat(lon).toFixed(6);
        }

        var fields = [
            _('System Name'),             "SystemName_s", systemName, "",
            _('Location (Lat,Lon)'),      "Lat_f",  location,   "",
            _('Connection Status'),       "",       connection, "",
            _('Signal Percentage'),       "",       percent,    "", 
            _('Cloud Connection Status'), "",       cloud_connection_state, "",
            _('Data Post Status'),        "",       post_state, "", 
            _('IMEI'),                    "IMEI_s", imei, "",
            _('Phone Number'),            "PhoneNumber_s", phonenumber, "",
            _('ICCID (SIM)'),             "ICCID_s",  iccid, "",
            _('APN (Access Point Name)'), "",         apn, "",
            _('Band'),                    "MCBV_s",     fb, "",
            homenetwork_label,            "HnName_s",   homenetwork, "", 
            _("TX Power"),                "TX_Power",     txpower,   "dBm", 
            _("CID (Serving Cell ID)"),   "ScId_i", cid, "",
            _("PCI (Physical Cell ID)"),  "ScPId_i", phycid, "",
            _("RSRQ (Reference Signal Received Quality)"),  "ScRsrq_i", rsrq, "dB", 
            _("RSRP (Reference Signal Received Power)"),    "ScRsrp_i", rsrp, "dBm",  
            _("RSSI (Received Signal Strength Indicator)"), "ScRssi_i", rssi, "dBm",  
            _("SINR (Signal to Interference+Noise Ratio)"), "ScSinr_f", snr,  "dB"
		];
     
		var fields_top = [
			_('Model'),            model,
            _('Serial Number'),    serialNumber,
//			_('Firmware Version'), firmware,
			_('Uptime'),           systeminfo.uptime ? '%t'.format(systeminfo.uptime) : null,
            // TODO: These values come from br-lan
            _("TX Bytes (since last power cycle)"), txbytes,
            _("RX Bytes (since last power cycle)"), rxbytes,
            //_('System Name'),             "SystemName_s", systemName, "",
            //_("IPv4 Address / IPv6 Address"),       lanip 
            //_("IPv4 Address"),       lanip   // Now configured in admin section    
		];
        
        var fields_mem = [
            _('Memory'), (mem.available) ? mem.available : (mem.total && mem.free && mem.buffered) ? mem.free + mem.buffered : null, mem.total
        ];
        
        var shutdown_val = uci.get('awc', 'mcu', 'shutdown_period');

        if (shutdown_val == "900")
            shutdown_val = "15 minutes"; 
        else if (shutdown_val == "3600")
            shutdown_val = "1 Hour";
        else
            shutdown_val = shutdown_val + " Seconds";
        
        var fields_bottom = [
          _("Vehicle Shutdown Timer"),            shutdown_val
        ];
        
		var table = E('table', { 'class': 'table' });

        this.fieldsAppendStatus(table, fields_top);
        this.fieldsAppendMem(table,    fields_mem); 
  //      this.fieldsAppendStatus(table, fields_bottom); // Now in the admin section.
		for (var field = 0; field < fields.length; field += 4) {
            var label = fields[field + 0];
            var name =  fields[field + 1];
            var value = fields[field + 2];
            var units = fields[field + 3];
            var style = '';
            
            if (!value) {
                value = "(Unknown)";
            }

            if (value == "(Unknown)" || value == "(Updating)") {
                style = "font-style: italic; color: grey";
                units = ""
            }
            
            if (name != "" && expiredNames[name] == true) {
                style = style + "; color: grey";
            }
            
            if (units != "") value = value + " " + units;
            
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '40%' }, [ label ]),
				E('td', { 'class': 'td left', 'style': style }, [ value ] )
			]));
		}  

      
		return table;
	}
});
