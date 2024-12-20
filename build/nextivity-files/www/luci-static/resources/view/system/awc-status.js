'use strict';
'require view';
'require form';
'require rpc';
'require ui';
'require uci';
'require fs';
'require tools.widgets as widgets';
'require validation';

var mapdata = { actions: {}, config: {} };
var cloudStateJson;

var pollURLold;
var pollURL;
var pollFreq;
var pollFreqold;

var statusURL;
var statusURLold;

var band, bandnew, bandAttempts;
var bandlock;

var intervalTimer;

return view.extend({

    handleFactoryReset: function() {
        if (!confirm(_('Do you really want to return settings to factory defaults?')))
            return;

        ui.showModal(_('Erasing...'), [
                        E('p', { 'class': 'spinning' }, _('The system is erasing the configuration partition now and will reboot itself when finished.'))
                ]);

        /* Will not return, hence no promise handling */
        fs.exec('/sbin/firstboot', [ '-r', '-y' ]);

        ui.awaitReconnect('192.168.113.1', 'openwrt.lan');
    },

    // Change UCI awc.main.bridge_mode setting
    handleMode: function(m, ev, mode) {

        // TODO: Should disable the logging toggle/turn off when Passthrough mode is enabled
        uci.set('awc', 'main', 'bridge_mode', mode); 
    },

    // Change UCI awc.main.logging setting
    handleLogging: function(m, ev, value) {

        /* 
        // This is no longer true. The reconnection happens in the monitoring scripts.
        if (value == "off") {
            var body = [];
            var force = E('input', { type: 'checkbox' });

            body.push(E('p', { 'class': 'alert-message'//,
                        }, 
                        [
                        _('Disabling logging may impact modem reconnection performance')
                        ]));

            ui.showModal(_('Logging Setting'), body);

            setTimeout(function() {
                ui.hideModal();
            }, (3000));
        }*/

        uci.set('awc', 'status_gather', 'logging', value); 
    },

    // Change UCI awc_cloud.cloud.status_host setting
    handleStatusURL: function(m, ev, value) {
        //console.log("URL: " + value);
    },

    // Change UCI awc_cloud.cloud.poll_host setting
    handlePollURL: function(m, ev, value) {
        //console.log("URL: " + value);
    },


    // Change UCI awc_cloud.cloud.action_poll setting
    handlePollFreq: function(m, ev, value) {
      //console.log("poll frequency: " + value);
    },

    // Change UCI awc.main.logging_push setting
    handlePush: function(m, ev, value) {
        //console.log("handlePush: " + value);
        uci.set('awc', 'status_gather', 'logging_push', value);
    },
        
    // Change UCI awc.main.logging_push setting
    handlePushPeriod: function(m, ev, value) {
        //console.log("handlePush: " + value);
        uci.set('awc', 'status_gather', 'period', value);
    },
    
    // Change UCI awc.main.logging_push setting
    handlePollPeriod: function(m, ev, value) {
        //console.log("handlePush: " + value);
        uci.set('awc', 'status_gather', 'poll', value);
    },
    
    // Change UCI awc.main.logging_gui setting
    handleGUI: function(m, ev, value) {
        //console.log("handleGUI: " + value);
        uci.set('awc', 'status_gather', 'logging_gui', value);
    },

    // Change UCI LAN IP setting
    handleIP: function(m, ev, value) {
        //console.log("handleIP: ", value);

        uci.set('network', 'lan', 'ipaddr', value);
        // network.lan.ipaddr
    },


    getButtonTitle: function(value) {
        if (value == "260" || value == "-1") {
            return _("Default Band Configuration");
        }

        return _("LTE B" + value + " Only");
    },


    // Change UCI MCU shutdown
    handleShutdown: function(m, ev, value) {
        //console.log("handleShutdown: " + value);

        uci.set('awc', 'mcu', 'shutdown_period', value);
    },


    handleBand: function(m, ev, value) {
        //console.log("handleBand: " + value);

        var buttonElement = document.getElementById("awc-band-button");
        if (buttonElement != undefined) {
           var oldText = this.getButtonTitle(band);
           var newText = this.getButtonTitle(value);

           var oldHTML = buttonElement.innerHTML;

           const newHTML = oldHTML.replace(oldText, newText);

           buttonElement.innerHTML = newHTML;
           // This destroys the onclick handler, so set again
           //buttonElement.onclick = L.bind(this.setBand, this);
        } else {
            console.log("Couldn't find band button");
        }

        //band = value;
        bandnew = value;
    },

    awaitBandChange: function() {
        //console.log("awaitBandChange: " + bandnew + " " + band);

        this.loadStatus();

        if (bandnew == band) {
           //console.log("Value changed to " + band);
           clearInterval(intervalTimer);
           ui.hideModal();
           this.handleBand(null, null, band);
           bandlock.default = band;

        } else if (++bandAttempts == 9) { // 45 seconds
           clearInterval(intervalTimer);
           ui.hideModal();

           //alert(_("Band configuration change timed out"));
        }

    },
    
    handleOffline: function(m, ev, value) {
//        console.log("handleOffline: " + value);
      
        uci.set('awc', 'main', 'reboot_offline', value);
    },

    setBand: function(m, ev, value) {
        var bandval = (bandnew == "260") ? "-1" : bandnew;

        //console.log("setBand: " + band + " " + bandval);
        var interrupt = _("This will cause a brief interruption of the connnection.");
        if (bandval != "-1") interrupt += " " + _("This setting will be lost when the MegaFi is power cycled.");

        if (!confirm(_("Set " + this.getButtonTitle(bandnew) + '?') + " " + interrupt))
            return;

        fs.exec("/sbin/set-band.sh", [ bandval ]);

        // TODO: Now wait until timeout or the value of "band" from polling changes from the current
        // value.

        ui.showModal(_('Updating Band Configuration...'), [
                     E('p', { 'class': 'spinning' }, _("Please wait"))
                     ]);

        bandAttempts = 0;
        intervalTimer = setInterval(L.bind(this.awaitBandChange, this), 5000, this);

    },

    // Change UCI API reboot
    handleAPIReboot: function(m, ev, value) {
        //console.log("handleAPIReboot");
        uci.set('awc', 'api', 'reboot', value);
    },

    handleAPIPowerCycle: function(m, ev, value) {
        //console.log("handleAPIPowerCycle");
        uci.set('awc', 'api', 'power_cycle', value);
    },
 
    handleAPIStatus: function(m, ev, value) {
        //console.log("handleAPIStatus");
        uci.set('awc', 'api', 'status', value);
    },

    handleLcdOrient: function(m, ev, value) {
        uci.set('lcd', 'settings', 'orientation', value);
    },

    handleLcdDetails: function(m, ev, value) {
        uci.set('lcd', 'settings', 'details', value);
    },

    handleLcdWorktime: function(m, ev, value) {
        uci.set('lcd', 'settings', 'worktime', value);
    },

    handleLcdSwitch: function(m, ev, value) {
        uci.set('lcd', 'settings', 'screen_switch', value);
    },
    
    handleGPSEnabled: function(m, ev, value) {
//        console.log("handleGPSEnabled")
        uci.set('speedway_gpsd', 'gpsd', 'enabled', value);
    },
    
    handleGPSIP: function(m, ev, value) {
  //      console.log("handleGPSIP")
        uci.set('speedway_gpsd', 'gpsd', 'client', value);
    },
    
    handleGPSPort: function(m, ev, value) {
        uci.set('speedway_gpsd', 'gpsd', 'port', value);
    },
    
    handleGPSFormat: function(m, ev, value) {
        uci.set('speedway_gpsd', 'gpsd', 'format', value);
    },   
    
    handleGPSTID: function(m, ev, value) {
        uci.set('speedway_gpsd', 'gpsd', 'tid', value);
    },   
    
    handleGPSRate: function(m, ev, value) {
        uci.set('speedway_gpsd', 'gpsd', 'rate', value);
    },   
     
    
    handleGPSProto: function(m, ev, value) {
        uci.set('speedway_gpsd', 'gpsd', 'tcp', value);
    },

    processModemResponse : function(values) {
        if (values == null) {
            //console.log("Modem values not fetched");
            return;
       }

       for (var index in values.status) {
            var entry = values.status[index];
            var name  = entry.name;
            var value = entry.value;

            if (name == "MCBV_s") {
                if (value.includes("0x", 0)) {
                  band = value.substring(2);
                } else {
                  band = value;
                }

//                console.log("Band = " + band);

                break;
            }
       }

       if (band == undefined) {
//           console.log("band value not found");
       } else {
//           console.log("band set to " + band);
       }
    },


    loadStatus : function() {
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
            L.resolveDefault(uci.load('awc_cloud')),
            L.resolveDefault(uci.load('awc_uuid')),
            L.resolveDefault(uci.load('speedway_gpsd')),
            L.resolveDefault(uci.load('network')),
            L.resolveDefault(uci.load('lcd')),
            L.resolveDefault(fs.read('/tmp/nextivity/cloud_state.json')),
            this.loadStatus(),
            ]).then(L.bind(function (data) {
                cloudStateJson = data[5];
            }, this))
    },
    
    render: function() {
        var m, s, o, ss;
        
        m = new form.JSONMap(mapdata, _('MegaFi Configuration'));

        // Appears to be needed since we are doing our own UCI handling
        m.save = function() {
//          console.log("Save");

          var statusURLval = statusURL.formvalue('actions');

//          console.log("vals: " + statusURLval + " " + statusURLold);

          if (statusURLval != statusURLold) {
            uci.set('awc_cloud', 'cloud', 'status_host', statusURLval);
            statusURLold = statusURLval;
          }

          var pollURLval = pollURL.formvalue('actions');

          if (pollURLval != pollURLold) {
            uci.set('awc_cloud', 'cloud', 'poll_host', pollURLval);
            pollURLold = pollURLval;
          }

          var pollFreqVal = parseInt(pollFreq.formvalue('actions'));

          if (pollFreqVal != pollFreqold) {
            uci.set('awc_cloud', 'cloud', 'action_poll', pollFreqVal);
            pollFreqold = pollFreqVal;
          }

          uci.save();
        }

        var could_uuid_val = uci.get('awc_cloud',  'cloud',   'uuid');
        var uuid_val    = uci.get('awc_uuid',  'uuid_params',   'uuid');
        if ((could_uuid_val != undefined) && (could_uuid_val != "")) {
            uuid_val = "";            
        }
        var status_val  = uci.get('awc',       'status_gather', 'influx_url');
        var poll_val    = uci.get('awc_cloud', 'cloud',         'poll_host');
        var pollf_val   = uci.get('awc_cloud', 'cloud',         'action_poll');

        s = m.section(form.NamedSection, 'actions', _('Cloud'));
        o = s.option(form.SectionValue, 'actions', form.NamedSection, 'actions', 'actions', _('Cloud'));
        ss = o.subsection;

        o = ss.option(form.Value, 'uuid', "UUID");

        o.monospace = true;
        o.cols      = 100;
        o.readonly  = true;
        o.editable  = false;
        o.default   = uuid_val;


        o = pollURL = ss.option(form.Value, 'pollurl', "Cloud Poll URL");

        o.cols      = 200;
        o.default   = pollURLold = poll_val;
        o.onchage   = this.handlePollURL;

        o = pollFreq = ss.option(form.Value, 'freq', "Cloud Poll Period (seconds)");

        o.default   = pollFreqold = pollf_val;
        o.datatype = 'and(min(30),max(86400))';

        o = statusURL = ss.option(form.Value, 'statusurl', "Cloud Status URL");

        o.cols      = 200;
        o.default   = statusURLold = status_val;
        o.onchage   = this.handleStatusURL;


        var cloud_connection_state = "(Unknown)";

        try {
            if (cloudStateJson) {
                var cloudState = JSON.parse(cloudStateJson);

                if (cloudState && cloudState.state) {
                    cloud_connection_state = cloudState.state;

                    if (cloud_connection_state == "Connected") {
                        var d = new Date(cloudState.time * 1000);
                        var cloud_date = d.toLocaleString();

                        cloud_connection_state = "Connected  (" + cloud_date + ")";
                    }
                }
            }

        } catch (exception) {
            //console.log("no cloud state");

        }

        var cloud_label = "Cloud Status";

        o = ss.option(form.DummyValue, 'cloudstate', cloud_label);

        o.default = cloud_connection_state;

        s = m.section(form.NamedSection, 'config', _('Logging'));
        o = s.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc', _('MegaFi Logging'));
        ss = o.subsection;

        var logging_val = uci.get('awc', 'status_gather', 'logging');

        o = ss.option(form.ListValue, 'logging', _('Logging Enabled'));
        o.value("on",  "Logging Enabled");
        o.value("off", "Logging Disabled");
        o.default  = logging_val;
        o.onchange = L.bind(this.handleLogging, this);

        var push_val = uci.get('awc', 'status_gather', 'logging_push');
        var gui_val  = uci.get('awc', 'status_gather', 'logging_gui');
        var push_period_val = uci.get('awc', 'status_gather', 'period');
        var poll_period_val = uci.get('awc', 'status_gather', 'poll');
        
        o = ss.option(form.ListValue, 'push', _('Push to Cloud'));
        o.value("on",  "Push Enabled");
        o.value("off", "Push Disabled");
        o.default  = push_val;
        o.onchange = L.bind(this.handlePush, this);
  
        o = ss.option(form.Value, 'push_period', _('Push to Cloud Period (seconds)'));
        o.datatype = 'and(min(15),max(86400))';
        o.default  = push_period_val;
        o.onchange = L.bind(this.handlePushPeriod, this);
  
        o = ss.option(form.Value, 'poll_period', _('System Poll Period (seconds)'));    
        o.datatype = 'and(min(15),max(120))';
        o.default  = poll_period_val;
        o.onchange = L.bind(this.handlePollPeriod, this);
        
        o = ss.option(form.ListValue, 'gui', _('Show in Local UI') /*, _('Enable or Disable local UI Publish')*/);
        o.value("on",  "Local UI Enabled");
        o.value("off", "Local UI Disabled");
        o.default  = gui_val;
        o.onchange = L.bind(this.handleGUI, this);

        var bridge_val = uci.get('awc', 'main', 'bridge_mode');

        o = s.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc', _('NAT or Passthrough Mode'));
        ss = o.subsection;

        o = ss.option(form.ListValue, 'bridge_mode', _('MegaFi Mode (Changing causes reboot)'));
        o.value("on",  "Passthrough Mode");
        o.value("off", "NAT Mode");
        o.default  = bridge_val;
        o.onchange = L.bind(this.handleMode, this);

        var lan_ip = uci.get('network', 'lan', 'ipaddr');

        o = ss.option(form.Value, 'ip_address', _('LAN IP Address'));
        o.datatype = 'ipaddr';
        o.onchange = L.bind(this.handleIP, this);
        o.default  = lan_ip;

        var reboot_offline_val = uci.get('awc', 'main', 'reboot_offline');
        
        //console.log("reboot_offline_val: " + reboot_offline_val);
        
        s = m.section(form.NamedSection, 'config', _('Modem'));
        o = s.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc', _('MegaFi and Modem Configuration'));
        
        ss = o.subsection;
        
        o = ss.option(form.ListValue, 'reboot_offline', _('Reboot Offline Time (minutes)'));
        o.value(0, "Disabled");
        o.value(180, "3");
        o.value(300, "5");
        o.value(600, "10");
        
        o.default = reboot_offline_val;
        o.onchange = L.bind(this.handleOffline, this);
        
        o = ss.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc');

        if (band == undefined) band = "260";
        bandnew = band;
        
        var status = this;
        
        o.render = L.bind(function(view, section_id) {
          //  console.log("Render band");  
          
          var m, s, o, ss;
        
          m = new form.JSONMap(mapdata);
          
          s = m.section(form.NamedSection, 'actions');
          o = s.option(form.SectionValue, 'actions', form.NamedSection, 'actions', 'actions' /*,  _('Modem Configuration')*/);
          ss = o.subsection;
          
          bandlock = o = ss.option(form.ListValue, 'bandlock', _('Band Lock'));
          o.value("14",  status.getButtonTitle("14"));
          o.value("260", status.getButtonTitle("260")); 
          o.default  = band;
          o.onchange = L.bind(status.handleBand, status);
          
          var table = E('table', { 'class': 'cbi-section' });
          var button =  E('button', {
                    'class': 'cbi-button cbi-button-apply',
                    'id' : 'awc-band-button',
                    'click' : L.bind(status.setBand, status),
            }, [ "Set " + status.getButtonTitle(band)]);
          
          m.render().then(function(node) {
            table.appendChild(E('tr', { 'class': 'tr' }, [
                E('td', { 'class': 'td left', 'width': '33%' }, [ node ]) ,
                E('td', { 'class': 'td left', 'width': '33%', 'valign': 'bottom' }, [ button ])
            ]));
          });
          
           return table;
        });
        
        var reboot_val      = uci.get('awc', 'api', 'reboot');
        var power_cycle_val = uci.get('awc', 'api', 'power_cycle');
        var status_val      = uci.get('awc', 'api', 'status');
        
        if (!reboot_val)           reboot_val = "disabled";
        if (!power_cycle_val) power_cycle_val = "disabled";
        if (!status_val)           status_val = "disabled";
        
        o = s.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc', _('API Configuration'));
        ss = o.subsection;

        o = ss.option(form.ListValue, 'reboot', _('MegaFi Reboot API Enabled'));
        o.value("enabled", "Enabled");
        o.value("disabled", "Disabled");
        o.default  = reboot_val;
        o.onchange = L.bind(this.handleAPIReboot, this);

        o = ss.option(form.ListValue, 'power_cycle', _('Modem Power Cycle API Enabled'));
        o.value("enabled", "Enabled");
        o.value("disabled", "Disabled");
        o.default  = power_cycle_val;
        o.onchange = L.bind(this.handleAPIPowerCycle, this);
        
        o = ss.option(form.ListValue, 'status', _('Modem Status API Enabled'));
        o.value("enabled", "Enabled");
        o.value("disabled", "Disabled");
        o.default  = status_val;
        o.onchange = L.bind(this.handleAPIStatus, this);

        // LCD configuration
        var lcd_orient_val      = uci.get('lcd', 'settings', 'orientation');
        var lcd_details_val     = uci.get('lcd', 'settings', 'details');
        var lcd_worktime_val    = uci.get('lcd', 'settings', 'worktime');
        var lcd_screen_switch_val    = uci.get('lcd', 'settings', 'screen_switch');

        if (!lcd_orient_val)        lcd_orient_val  = "landscape";
        if (!lcd_details_val)       lcd_details_val = "full";
        if (!lcd_worktime_val)      lcd_worktime_val = -1;
        if (!lcd_screen_switch_val) lcd_screen_switch_val = 10;

        o = s.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc', _('LCD Configuration'));
        ss = o.subsection;

        o = ss.option(form.ListValue, 'orientation', _('Screen Orientation'));
        o.value("landscape", "Landscape");
        o.value("portrait", "Portrait");
        o.default  = lcd_orient_val;
        o.onchange = L.bind(this.handleLcdOrient, this);

        o = ss.option(form.ListValue, 'details', _('Detail Level'));
        o.value("full", "Full");
        o.value("minimal", "Minimal");
        o.default  = lcd_details_val;
        o.onchange = L.bind(this.handleLcdDetails, this);

        o = ss.option(form.MultiValue, 'worktime', _('Turn off screen after (seconds)'));
        o.datatype = 'range(-1, 3600)';
        o.value(-1, "Always Off");
        o.value(0, "Always On");
        o.create = true;
        if (lcd_worktime_val == 0) {
            o.default = "Always On";
        } else if (lcd_worktime_val == -1) {
            o.default = "Always Off";
        } else {
            o.default  = lcd_worktime_val;
        }
        o.onchange = L.bind(this.handleLcdWorktime, this);
        o.multiple    = false;
        o.placeholder = false;

        o = ss.option(form.Value, 'screen_switch', _('Switch screen information (seconds)'));
        o.default  = lcd_screen_switch_val;
        o.datatype = 'and(min(1),max(60))';
        o.onchange = L.bind(this.handleLcdSwitch, this);
        
/*        
        var gps_enabled_val     = uci.get('speedway_gpsd', 'gpsd', 'enabled');
        var gps_client_val      = uci.get('speedway_gpsd', 'gpsd', 'client');
        var gps_port_val        = uci.get('speedway_gpsd', 'gpsd', 'port');
        var gps_format_val      = uci.get('speedway_gpsd', 'gpsd', 'format');
        var gps_proto_val       = uci.get('speedway_gpsd', 'gpsd', 'tcp');
        var gps_tid             = uci.get('speedway_gpsd', 'gpsd', 'tid');
        var gps_rate            = uci.get('speedway_gpsd', 'gpsd', 'rate');
        
        if (!gps_enabled_val)   gps_enabled_val = "disabled";
        if (!gps_port_val)      gps_port_val    = 5555;
        
        o = s.option(form.SectionValue, 'config', form.NamedSection, 'config', 'awc', _('GPS Send Configuration'));
        ss = o.subsection;

        o = ss.option(form.ListValue, 'gpsd_enabled', _('Send GPS Data to Remote Host'));
        o.value("true", "Enabled");
        o.value("false", "Disabled");
        o.default  = gps_enabled_val;
        o.onchange = L.bind(this.handleGPSEnabled, this);

        o = ss.option(form.Value, 'gpsd_ip', _('Client IP Address'));
        o.default  = gps_client_val;
        //o.datatype = 'and(min(1),max(65535))'
        o.onchange = L.bind(this.handleGPSIP, this);

        o = ss.option(form.Value, 'gpsd_port', _('TCP/UDP Port'));
        o.default  = gps_port_val;
        o.datatype = 'and(min(1),max(65535))';
        o.onchange = L.bind(this.handleGPSPort, this);

        o = ss.option(form.ListValue, 'gpsd_format', _('Format'));
        o.value("nmea", "NMEA");
        o.value("taip", "TAIP");
        o.default  = gps_format_val;
        o.onchange = L.bind(this.handleGPSFormat, this);
        
        o = ss.option(form.Value, 'gpsd_tid', _('TAIP ID'));
        o.default  = gps_tid;
        o.datatype = 'or(length(0),length(4))';
        o.onchange = L.bind(this.handleGPSTID, this);
        
        o = ss.option(form.ListValue, 'gpsd_proto', _('TCP/UDP'));
        o.value("true",  "TCP");
        o.value("false", "UDP");
        o.default  = gps_proto_val;
        o.onchange = L.bind(this.handleGPSProto, this);

        o = ss.option(form.Value, 'gpsd_rate', _('Send Period (seconds)'));
        o.default  = gps_rate;
        o.datatype = 'and(min(0),max(3600))';
        o.onchange = L.bind(this.handleGPSRate, this);
  */      
        s = m.section(form.NamedSection, 'actions', _('Actions'));
        o = s.option(form.SectionValue, 'actions', form.NamedSection, 'actions', 'actions', _('Configuration'));
        ss = o.subsection;

        o = ss.option(form.Button, 'reset', _('Restore Configuration'));
        o.inputstyle = 'action important';
        o.inputtitle = _('Factory Defaults');
        o.onclick = L.bind(this.handleFactoryReset, this);
        
        s = m.section(form.NamedSection, 'actions', _('Serial Number'));
        o = s.option(form.SectionValue, 'actions', form.NamedSection, 'actions', 'actions', _('Serial Number'));
        ss = o.subsection;
        
        o = ss.option(form.DummyValue, 'serial'); 
        
        o.load = function(section_id) {
            return L.resolveDefault(fs.read('/tmp/nextivity/serialNumber'), '');
        };
        
        s = m.section(form.NamedSection, 'actions', _('Build Information'));
        o = s.option(form.SectionValue, 'actions', form.NamedSection, 'actions', 'actions', _('Build Information'));
        ss = o.subsection;
        
        o = ss.option(form.TextValue, 'build');
        o.cols = 128;
        o.rows = 11;
        o.width = 800;
        o.monospace = true;
        o.readonly = true;
        o.editable = false;
        
        o.load = function(section_id) {
            return L.resolveDefault(fs.read('/etc/nextivity_build_info'), '');
        };
        
        return m.render();        
    }
});
