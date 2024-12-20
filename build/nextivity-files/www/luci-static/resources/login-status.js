'use strict';
'require baseclass';
'require fs';
'require ui';
'require rpc';
'require uci';
'require network';


var intervalTimer;
var refresh_time = 10 * 1000;
var fields = [ "connect", "signal", "band", "apn", "wanip", "lanip" ];


return baseclass.extend({

   
  __init__: function() {
//      console.log("login-status init");
    
      this.render();
      
      /*this.load().then(L.bind(this.render, this)).catch(function(exception) {
            console.log("Error during login-status init");
       });*/
  },
  
  loadModemStatus: function() {
  
    //console.log("login-status: loadModemStatus");
    
    L.resolveDefault(fs.read('/tmp/nextivity/modemstatus.json')
      .catch(L.bind(function(err) { console.error(err); } ))
    
    ).
       then(L.bind(function (status) {
         if (status == undefined) {
           //console.error("login-status: failed to load modemstatus");
           this.populateStatus(null);
         } else {
            this.populateStatus(JSON.parse(status));
         }
       
       }, this))
      .catch(L.bind(function (err) {
          //console.log("login-status: Error during modemstatus.js loading", err);
          this.populateStatus(null);
          
    }, this));  
      
    //console.log("login-status: loadModemStatus done");
  },

  
  populateStatus: function(modemStatus) {
    var lanIP; 
    var wanIP;
    var homeNetwork;
    var fb = "--";
    var rsrq, rsrp;
    var sigperc;
    var apn;
    var connection; 
    var noSim = false;
    
    if (modemStatus && modemStatus.status) {
        for (var index in modemStatus.status) {
            var entry = modemStatus.status[index];
            var name  = entry.name;
            
            if (name == "PdpIpAddr_s") {
                wanIP = entry.value;
                
            } else if (name == "LanIpAddr_s") {
                lanIP = entry.value
                
            } else if (name == "HnName_s") {
                homeNetwork = entry.value;
                
            } else if (name == "ScFb_i") {
               fb = entry.value;   
             
            } else if (name == "ScRsrq_i") {
               rsrq = entry.value;
               
            } else if (name == "ScRsrp_i") {
               rsrp = entry.value;
            
            } else if (name == "SignalPercent_s") {
               sigperc = entry.value.replace("%","");               
            
            } else if (name == "Apn_s") {
               apn = entry.value;
               
            } else if (name == "Connection_s") {
               connection = entry.value;
        
               if (connection == "SIM Missing") noSim = true;
            }
        }
    }
    
    if (noSim) fb = "--";
    
    //var signal = getSignalFromRSRP(rsrp, homeNetwork);
    var signal = getSignalFromPercent(sigperc, homeNetwork);
    
    for (var field = 0; field < fields.length; field++) {
        var name = fields[field];
        var text = "loginstatus-" + name + "-text";
        var icon = "loginstatus-" + name + "-icon";
        var aux  = "loginstatus-" + name + "-aux";
      
        var textElement = document.getElementById(text);
        var iconElement = document.getElementById(icon);
        var auxElement  = document.getElementById(aux);
      
        if (!textElement || !iconElement) {
            console.log("Elements not found for '" + name + "'");
            continue;
        }
      
        switch (name) {
            case "connect":
                // If WAN IP is set and signal, assume connected
              
                if (!noSim && wanIP != undefined && (signal.percent > 0)) {
                    textElement.innerHTML = _("Connected");
                    iconElement.innerHTML = getIcon("connected-cir.svg", 75);
                    auxElement.innerHTML  = (homeNetwork == undefined) ? "(Unknown)" : homeNetwork;
                    
                } else {
                    textElement.innerHTML = _("Not Connected");    
                    iconElement.innerHTML = getIcon("disconnected-cir.svg", 75);
                    auxElement.innerHTML  = "";
                    
                }                
                
                break;
              
            case "signal":
                textElement.innerHTML = _("Signal Strength");
                
                //var signal = getSignalFromRSRP(rsrp, homeNetwork);
                
                if (noSim) {
                    iconElement.innerHTML = "(No SIM)";
                    auxElement.innerHTML = "";
                } else {
                    iconElement.innerHTML = getIcon("signal-" + signal.percent + ".svg", 75);
                    auxElement.innerHTML  = "(" + signal.text + ")";
                }
                
                break;
              
            case "band":
                textElement.innerHTML = _("Band");
                iconElement.innerHTML = fb;
                break;
          
            case "apn":
                textElement.innerHTML = _("Access Point Name");
                
                if (noSim) {
                    iconElement.innerHTML = "(No SIM)";
                    auxElement.innerHTML = "";
                } else {
                    if (apn == undefined) apn = "";
                
                    if (apn == "firstnet-broadband" || homeNetwork == "FirstNet") {
                        iconElement.innerHTML = getIcon("firstnet.jpg", 75);
                    } else {
                        iconElement.innerHTML = getIcon("skin-logo.svg", 75);
                    }
                
                    auxElement.innerHTML = apn;
                }
                
                break;
             
            case "wanip":
                textElement.innerHTML = _("WAN IP Address");
              
                if (wanIP != undefined && !noSim) {
                    iconElement.innerHTML = wanIP;
                } else {
                    iconElement.innerHTML = _("(Not set)");
                }
                break;
             
            case "lanip":
                textElement.innerHTML = _("Local IP Address");
              
                if (lanIP != undefined) {
                    iconElement.innerHTML = lanIP;
                } else {
                    iconElement.innerHTML = _("(Not set)");
                }
                break;
        }
    }
  },


  render: function(data) {
     
    L.bind(this.loadModemStatus, this)();
        
    intervalTimer = setInterval(L.bind(this.loadModemStatus, this), refresh_time); 
  }
  

});

