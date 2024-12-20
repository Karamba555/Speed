'use strict';
'require view';
'require uci';
'require fs';

// TODO: This page stops display of the indicators. It's not clear why.
//

var modemValues = new Map();
var pageReady = false;
var intervalTimer;
var refresh_page  = 3;  // Page refresh time
var refresh_time  = 60; // Refresh of JSON
var lastFetch;

modemValues.set('Connection_s',     { label: "LTE Connection state" });
modemValues.set('SignalPercent_s',  { label: "Signal Percentage" });
modemValues.set('WWanTxBytes_i',    { label: "TX Bytes" });
modemValues.set('WWanRxBytes_i',    { label: "RX Bytes" });
modemValues.set('WWanTxPkts_i',     { label: "TX Packets" });
modemValues.set('WWanRxPkts_i',     { label: "RX Packets" });
modemValues.set('SystemName_s',     { label: "System Name" });
modemValues.set('ModemSwVersion_s', { label: "Modem Software Version" });
modemValues.set('IMEI_s',         { label: "IMEI" });
modemValues.set('PhoneNumber_s',  { label: "Phone Number" });
modemValues.set('ICCID_s',        { label: "ICCID" });
modemValues.set('IMSI_s',         { label: "IMSI" });
modemValues.set('ScMode_s',       { label: "Mode" });
modemValues.set('ScId_i',         { label: "ID" });
modemValues.set('ScPId_i',        { label: "PID" });
modemValues.set('ScEarfcn_i',     { label: "EARFCN" });
modemValues.set('ScFb_i',         { label: "Band" });
modemValues.set('ScTac_i',        { label: "TAC" });
modemValues.set('ScRsrp_i',       { label: "RSRP", units: "dBm" } );
modemValues.set('ScRsrq_i',       { label: "RSRQ", units: "dB"  } );
modemValues.set('ScRssi_i',       { label: "RSSI", units: "dBm" } );
modemValues.set('ScSinr_f',       { label: "SINR", units: "dB"  } );
modemValues.set('TX_Power',       { label: "TX Power", units: "dBm" });

modemValues.set('Apn_s',          { label: "APN" });
modemValues.set('ScMcc_i',        { label: "Home Network MCC" });
modemValues.set('ScMnc_i',        { label: "Home Network MNC" });
modemValues.set('HnName_s',       { label: "Home Network Name" });

modemValues.set('Lat_f',          { label: "Latitude" });
modemValues.set('Lon_f',          { label: "Longitude" });
modemValues.set('NMEA_s',         { label: "NMEA/TAIP Position" });
modemValues.set('GpsTime_s',      { label: "GPS UTC Timestamp" }); // This value is calcuated here, not from JSON

return view.extend({
    
    /**
     * Update HTML for each item. For this method, special case
     * any formatting - highlighting, colours, rounding, unset values, etc. 
     */

    updateModemHTML : function() {
        //console.log("updateModemHTML");
	
        for (let [entry, values] of modemValues) {
            var element = document.getElementById(entry);
		
            //console.log("Update: " + entry + " " + values);
		
            if (element) {
                var value = values.value;
                var units = values.units;
                 
                //console.log("Value: " + value);
			
                var isUndef = true;
                
                if (value === undefined) {
                    if (values.oldValue != undefined) {
                       value = "<span style=\"color:grey\"><i>" + values.oldValue + 
                         ((units != null) ? (" " + units) : "") + 
                         "</i></span>";  
                        
                    } else if (entry == "TX_Power") {
                        value = "<span style=\"color:grey\"><i>Updating</i></span>";
                        
                    } else {
                        value = "<span style=\"color:grey\"><i>Unknown</i></span>";
                        
                    }
                } else {
                  isUndef = false; 
                }
                
                if (units != null && !isUndef) {
                    value += " " + units; 
                }
			
                element.innerHTML = value; 
            } else {
                //console.log("Element '" + entry + "' not found");	
            }
        }
    },
	
    expireData: function() {
  //      console.log("Expiring data");
        
        for (let [currEntry, values] of modemValues) {	
           values.value = undefined;   
        }
        
        this.updateModemHTML();
    },
    
    setReady: function(readyElement, ready) {
        if (!readyElement) return;
                   
        if (ready) {
            readyElement.innerHTML = "";     
        } else {
            readyElement.innerHTML = "<span style=\"color:grey\"><i>Waiting for modem status refresh</i></span><p>"; 
        }
    },

    processModemResponse : function(values) {

        var ready = false;
        var readyElement = document.getElementById("modemStatusReady");
    
        if (readyElement) {
            ready = true;
        }

        this.setReady(readyElement, ready && ((values != null) || (lastFetch != undefined)));
        if (values == null) return;
  
        /* For each value we receieved, update/add in our copy of values.
            Note that we are turning an indexed array into a Javascript map. */
        var updated = false;
        var awc_gpsd_internal_format = uci.get('awc_gpsd', 'internal', 'format');
        
        for (var index in values.status) {
            var entry          = values.status[index];
            var name           = entry.name;
            var value          = entry.value;
            var itemLastUpdate = entry.lastUpdate;

            if (itemLastUpdate == -1) {
                // Value has expired
                value = undefined;
            }  else if (name == "ScSinr_f") { 
                value = parseFloat(value).toFixed(1);              
            }
			   
            var found = false;
			
            for (let [currEntry, values] of modemValues) {				
                if (currEntry == name) {
                    var oldValue = values.value;
					
                    if (value == undefined) {
                        //console.log("Value for " + name + " expired");
                        if (oldValue != undefined) values.oldValue = oldValue;
                        
                        values.value = value;	
                        updated = true;
                        
                    } else if (oldValue != value) {
					    //console.log("Value for " + name + " changed from " + oldValue + " to " + value);
                        values.value = value;	
                        updated = true;
                    }
                    found = true;
                    
                    if ((name == "NMEA_s") && (awc_gpsd_internal_format != 'disabled')) {
                      var timeStamp = modemValues.get('GpsTime_s'); 
                        
                      var date = new Date(entry.lastUpdate * 1000);
                      
                      timeStamp.value = date.toString();// "%t".format(entry.lastUpdate);
                    }
                    
                    if ((name == "Lat_f") || (name == "Lon_f") || (name == "NMEA_s") || (name == "NMEAGpsTime_s_s")) {
                        if (awc_gpsd_internal_format == 'disabled'){
                            value = undefined;
                            values.value = value;
                            updated = true;
                        }               
                    }
                    break;
                }
            }

        }
		
        // Now update the HTML
        if ((!pageReady && ready) || updated) {
            this.updateModemHTML();
        
            if (!pageReady && ready) {
//              console.log("Page now ready");
                pageReady = true;
                clearInterval(intervalTimer);
                intervalTimer = setInterval(L.bind(this.loadStatus, this), refresh_page * 1000);
            }
        } else {
  //     console.log("No values updated");   
        }
    },
    
  
	modemStatusRow : function(entry, lwidth = undefined, dwidth = undefined) {
	  var row = "";
	  var values = modemValues.get(entry);
	  var label;
	  
      if (lwidth == undefined)
        lwidth = "width='50%'"
      else
        lwidth = "width='" + lwidth + "'";
      
      if (dwidth == undefined)
        dwidth = "width='50%'"
      else
        dwidth = "width='" + dwidth + "'";
      
	  if (values == null) {
		  label = "<i>Unknown</i>"
	  } else {
		  var text  = values.label;
		  
		  if (text == null) text = entry;
                   
	      label = "<span style='font-weight:bold'>" + text + "</span>";
	  }
	  
	  row += "<tr><td " + lwidth + ">" + label + "</td><td " + dwidth + "><div id='" + entry + "'></div></td></tr>";
	  
	  return row;
	},
	
	modemStatusTable : function() {
	  var status = "";
      var tableStart = "<table width='100%' border='1'>";
	  var tableEnd   = "</table>";
	  var rule       = "<hr>";
	  var rowStart   = "<tr><td valign='top' width='50%'>";
      var rowStart2  = "<tr><td valign='top' width='50%' rowspan='2'>";
      var rowStart3  = "<tr><td valign='top' width='100%' colspan='2'>";
	  var colBreak   = "</td><td valign='top' width='50%'>";
      var colBreak2  = "</td><td valign='top' width='50%' rowspan='2'>";
	  var rowEnd     = "</td></tr>";
	  
      status += "<h2>Modem Status</h2>";
	  status += "<hr class='title-section'>";
      
      status += "<div id='modemStatusReady'></div>";
      
	  // Outer table
	  status += tableStart + rowStart;
	  
	  // Inner table, two columns, first row
      status += tableStart;
	  
	  var section = [ 'SystemName_s', 'ModemSwVersion_s', 'IMEI_s', 'ICCID_s', 'IMSI_s', 'PhoneNumber_s' ];
      
	  for (var entry = 0; entry < section.length; entry++) {
		status += this.modemStatusRow(section[entry]);  
	  }
	  
	  status += tableEnd + colBreak + tableStart;
	  
	  section = [ 'Connection_s', 'SignalPercent_s',
                  'WWanTxBytes_i', 'WWanRxBytes_i', 'WWanTxPkts_i', 
	              'WWanRxPkts_i' ];
	  
	  for (var entry = 0; entry < section.length; entry++) {
		status += this.modemStatusRow(section[entry]);  
	  }

	  status += tableEnd + rowEnd;

      // Second row, two columns
      
      status += rowStart3 + tableStart;
      
      var section = [ 'Lat_f', 'Lon_f', 'NMEA_s', 'GpsTime_s' ];
      
	  for (var entry = 0; entry < section.length; entry++) {
		status += this.modemStatusRow(section[entry], "25%", "75%");  
	  }

      status += tableEnd + rowEnd;
      
	  // Third row
	  status += rowStart + tableStart;
	  
      section = [ 'ScMode_s', 'ScId_i', 'ScPId_i', 'ScEarfcn_i','ScMcc_i', 'ScMnc_i', 'HnName_s' ];
      
      for (var entry = 0; entry < section.length; entry++) {
		status += this.modemStatusRow(section[entry]);  
	  }
      
      // This goes over two rows
	  status += tableEnd + colBreak2 + tableStart;
	  
	  section = [ 'Apn_s',  'ScFb_i', 'ScTac_i', 'ScRsrp_i', 'ScRsrq_i', 'ScRssi_i', 'ScSinr_f','TX_Power' ];
	  
	  for (var entry = 0; entry < section.length; entry++) {
		status += this.modemStatusRow(section[entry]);  
	  }
	  
	  return status;
	},
	
    loadStatus: function() {
       L.resolveDefault(fs.read('/tmp/nextivity/modemstatus.json')).
       then(L.bind(function (status) {
         this.processModemResponse(JSON.parse(status));
         
       }, this))
       .catch(L.bind(function (err) {
          console.log("Error during modemstatus.js loading", err);
         this.processModemResponse(null);
          
       }, this));
    },

	load: function() {
		return Promise.all([
			L.resolveDefault(uci.load('awc')),
			L.resolveDefault(uci.load('awc_gpsd')),
			this.loadStatus()]
		);
	},
	
	render: function() {
        refresh_time = uci.get('awc', 'status_gather', 'period');
        
        intervalTimer = setInterval(L.bind(this.loadStatus, this), 2000);
        
        return this.modemStatusTable();
	},
	
	addFooter: function() {

	}
});
