'use strict';
'require baseclass';
'require fs';
'require ui';
'require rpc';
'require uci';
'require network';


var intervalTimer;
var refresh_time = 10 * 1000;
var firmware_version = "(Unknown)";
var update_version;
var config_updates = false;

return baseclass.extend({
	__init__: function() {
      //console.log("header-status init");
	   this.load().then(L.bind(this.render, this)).catch(function(exception) {
            console.log("Error during init");
       });
	},

    load: function() {
       
        var session = rpc.getSessionID();
        var validSession = !session.startsWith("00000000");

        //console.log("header-status Load: " + session);
        
        try {
        
            return Promise.all([
              L.resolveDefault(validSession ? uci.load('network') : null),
              L.resolveDefault(validSession ? uci.load('awc') : null),
              L.resolveDefault(fs.read('/etc/nextivity_build_info')).catch(function(exception) {
                console.log("Error during loading build.info");
              })

            ]).catch(function(exception) {
              console.log("Error during load");
            })

        } catch (exception) {
          console.log("Exception during load promise");
        }

        //console.log("Done load");
    },


    loadModemStatus: function() {

      try {

        L.resolveDefault(fs.read('/tmp/nextivity/modemstatus.json')/*.catch(L.bind(function(err) { console.log("catch 1"); } ))*/).
        then(L.bind(function (status) {

          if (status == undefined) {
            //console.log("header-status: loadModemStatus failed to load");
            this.populateStatus(null);
          } else {
            this.populateStatus(JSON.parse(status));
          }

        }, this)).catch(L.bind(function (err) {
          console.log("Error during modemstatus.js loading", err);
         this.populateStatus(null);

       }, this)); 

       } catch (exception) {
          console.log("Exception during promise");
       }       
    },


    handleUpdate: function () {
        var ip = L.uci.get('network', 'lan', 'ipaddr');

        if (ip == undefined) ip = '192.168.113.1';
    
        if (update_version != undefined) {
//       console.log("handleUpdate firmware");
    
            if (!confirm(_('Apply firmware update now?'))) {
                return;
            }
       
            L.ui.showModal(_('Flashing…'), [
					E('p', { 'class': 'spinning' }, _('The system is updating the firmware now'))
				]); 
    
            L.fs.exec('/sbin/wakeup-cloud.sh "upgrade"');
    
            var timeout = 60 * 2;
      
            setTimeout(function() {
        //L.ui.hideModal();
        
                L.ui.awaitReconnect(window.location.host, ip, 'openwrt.lan');
            }, (timeout * 1000));

       
        } else if (config_updates) {
//       console.log("handleUpdate config");
      
            if (!confirm(_('Apply configuration update now?'))) {
                return;
            }
    
            L.ui.showModal(_('Reloading…'), [
					E('p', { 'class': 'spinning' }, _('The system is applying configuration now'))
				]); 
    
            L.fs.exec('/sbin/wakeup-cloud.sh "config"');
    
            var timeout = 10;
      
            setTimeout(function() {
         //L.ui.hideModal();
        
                L.ui.awaitReconnect(window.location.host, ip, 'openwrt.lan');
            }, (timeout * 1000));
      
       
        } else {
  //     console.log("handleUpdate nothing to do");   
            return;
        }   

    },

    populateStatus: function(modemStatus) {
        var homeNetwork;
        var rsrq, rsrp;
        var sigperc;
        var fb;
        var apn;
        var protocol;
        var serialNumber = "(Unknown)";
        var macAddress   = "(Unknown)";

//        console.log("populateStatus: " + modemStatus.status);
 
        var modeReport = document.getElementById("mode-reporting");
        
        var bridge_mode = L.uci.get('awc', 'main', 'bridge_mode');
        // Access denied
        if (!bridge_mode) {
          var protocol = "Unknown";

          // Report if MBIM mode
          if (modemStatus && modemStatus.status) {
            for (var index in modemStatus.status) {
              var entry = modemStatus.status[index];
              var name  = entry.name;

              if (name == "Protocol_s") {
                protocol = entry.value;
                if (protocol == "qmi") {
                  protocol = "QMI";
                } else  if (protocol == "mbim") {
                  protocol = "MBIM";
                } else {
                  protocol = "Unknown";
                }

              } else if (name == "Sn_s") {
                serialNumber = entry.value; 
                                  
              } else if (name == "LanMacAddr_s") {
                macAddress = entry.value.toUpperCase();
                  
              }
            }
          }

          if (modeReport) {
            if (protocol == "MBIM") {
               var warning = "Modem Protocol: " + protocol;
               modeReport.innerHTML = warning;

            } else {
               modeReport.innerHTML = "";

            }
          }
          
/*
          var serialElement = document.getElementById("serial-number");
          if (serialElement) {
             serialElement.innerHTML = "Serial Number: " + serialNumber; 
          }
          
          var macElement = document.getElementById("lan-mac");
          if (macElement) {
             macElement.innerHTML = "MAC Address: " + macAddress; 
          }
*/

          return;
        }

        L.fs.read('/etc/awc/config_updates').then(function(config_updates_data) {
            console.log("Config updates available: " + config_updates_data);
            config_updates = (config_updates_data != undefined);
        }).catch(function(exception) {
            config_updates = false;
              //console.log("Config updates not available");
              //console.error(exception);

        });


        if (modemStatus && modemStatus.status) {
            for (var index in modemStatus.status) {
                var entry = modemStatus.status[index];
                var name  = entry.name;
        
  //              console.log("Name: " + name);
                
                if (name == "HnName_s") {
                    homeNetwork = entry.value;
                
                } else if (name == "ScFb_i") {
                    fb = entry.value;   
               
                } else if (name == "ScRsrq_i") {
                    rsrq = entry.value;
               
                } else if (name == "ScRsrp_i") {
                    rsrp = entry.value;
                 
                } else if (name == "Apn_s") {
                    apn = entry.value;
                
                } else if (name == "SwVersion_s") {
                    firmware_version = entry.value;
            
                } else if (name == "SignalPercent_s") {
                    sigperc = entry.value.replace("%","");               
                                
                } else if (name == "Protocol_s") {
                    protocol = entry.value; 
                    //console.log("Protocol: " + protocol);
                    

                }
            }
        }
    
        if (modeReport) {
            //var bridge_mode = L.uci.get('awc', 'main', 'bridge_mode');
            var message;
            
            //if (!bridge_mode) {
            //    console.log("Warning: Networking mode not determined");
            //    return;
                //message = "";
            //} else {
              //console.log("Passthrough mode: " + bridge_mode);
                message = (bridge_mode == "on") ? "Passthrough" : "NAT";
            //}
       
            var html = "Network Mode: " + message; 
       
            //html += "</i><br><i>";
       
            var location = window.location.pathname; 
       
            //if (location == "/cgi-bin/luci/" || location == "/cgi-bin/luci/admin/awc/overview") {
            //    html += "Overview";
            //} else {
            //    html += "Expert Mode";
            //}
       
            //html += "</i>";
       
            modeReport.innerHTML = html;
        }

        var logo = document.getElementById("skin-logo");
        if (logo) {
          var logoImage = getIcon("skin-logo.svg", 200);

          var html = "<div id='skin-logo'>" + logoImage;
                          
          logo.innerHTML = html;
        }
    
        var antenna = document.getElementById("header-signal");
    
        if (antenna) {
            //var signal = getSignalFromRSRP(rsrp, homeNetwork);
            var signal = getSignalFromPercent(sigperc, homeNetwork);
        
            var antennaImage = getIcon("antenna.png", 50);
            var signalImage  = getIcon("signal-" + signal.percent + ".svg", 50);

            var html = "<span id='antenna-image'>" + antennaImage;

            if (fb) {
                html += "<span class='header-antenna-band'>" + fb + "</span>";
            }
        
            html += "</span>" + signalImage;
                            
            antenna.innerHTML = html;
        }     


        L.fs.read('/etc/awc/software_updates').then(function(software_updates) {
        //console.log("software_updates: " + software_updates);
            var actions = JSON.parse(software_updates);
    
            if (actions && actions["ei.action"]) {
                var update = actions["ei.action"];
                var value = update["software.update"];
       
                if (value) {
                    update_version = value.replace("repo/code/", "");
                }
            }
        }).catch(function(error) {
            //console.error(error);
        
            update_version = undefined;
        
        });
        
        /*var software_version = document.getElementById("software-version");
    
        if (software_version) {
            software_version.onclick = L.bind(this.handleUpdate, this);

            var html = "Firmware Version: " + firmware_version;
            var message_update;

            if (update_version) {
                message_update = "Firmware Update Available: " + update_version;

            } else if (config_updates) {
                message_update = "Configuration Updates Pending";

            }

            if (message_update != undefined) {
                html += '<br><span style="background: var(--secondary-dark-color);' +
                           'color: var(--main-bright-color);' +
                           'font-style: italic;' +
                            '">' + message_update + '</span>'
            }
            
            //console.log("html:" + html);
            //console.log("message: " + message);

            //software_version.innerHTML = html;
        } else {
            console.log("Software version div not found");
        }*/
                
    },


    render: function(data) {
        var build_info = data[2];

        if (build_info) {
            var match = "Firmware Version: ";
            var index = build_info.indexOf(match);

            if (index != -1) {
                var version = build_info.substr(index + match.length);
                var split = version.split("\n");

                if (split && split[0]) {
                    firmware_version = split[0];
                    //console.log("Firmware Version from build: '" + firmware_version + "'");
                }
            }
        }
        
        L.bind(this.loadModemStatus, this)();
        
        intervalTimer = setInterval(L.bind(this.loadModemStatus, this), refresh_time);   
    }

});


