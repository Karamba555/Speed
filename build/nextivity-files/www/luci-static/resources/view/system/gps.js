'use strict';
'require view';
'require form';
'require tools.widgets as widgets';

var mapdata = { actions: {}, config: {} };

return view.extend({
	render: function() {
		var m, s, ss, o;
        
        //m = new form.JSONMap(mapdata, _('GPS Server Configuration'));
        
/*        m.save = function() {
        
        }  */
                
        
		m = new form.Map('speedway_gpsd', _('GPS Output Configuration'), _('Configure GPS output in NMEA and TAIP format to hosts'));
      
        s = m.section(form.TypedSection, 'server', _('GPS Server'));
        s.anonymous = true;
        
        o = s.option(form.Value, 'serverPort', "Server Port");
        
        var serverPort = L.uci.get('speedway_gpsd', 'server', 'serverPort');
        
        o.default = serverPort;
        o.datatype = 'or("",and(min(1024),max(65536)))';
        
        
        s = m.section(form.TypedSection, 'internal', _('GPS Internal Reporting'));
        s.anonymous = true;

        o = s.option(form.ListValue, 'format', _('Output Format'), _('Specify NMEA or TAIP output'));
        o.value("disabled", "Disabled");
        o.value("nmea", "NMEA");
        o.value("taip", "TAIP");
		o.rmempty  = false;

        o = s.option(form.Value, 'tid', _('NMEA station code or TAIP ID'));
		o.datatype    = 'or(length(0),length(4))';
		o.placeholder = '';
        o.rmempty = true;

        o = s.option(form.Value, 'rate', _('Rate'), _('Optional rate limit in seconds'));
		o.datatype    = 'rate';
        o.datatype    = 'and(min(0),max(3600))';
		o.placeholder = 1;
        
        
		s = m.section(form.TypedSection, 'gpsd', _('GPS Output'));
		s.anonymous = true;
		s.addremove = true;
		s.addbtntitle = _('Add output');

        o = s.option(form.Value, 'client', _('Host IP Address'));
		o.datatype    = 'ipaddr';
		o.placeholder = '';
        
		o = s.option(form.Value, 'port', _('Port'));
		o.datatype    = 'port';
        o.datatype    = 'and(min(1),max(65535))';
		o.placeholder = 5555;
        o.rmempty = false;

		o = s.option(form.ListValue, 'format', _('Output Format'), _('Specify NMEA or TAIP output'));
        o.value("nmea", "NMEA");
        o.value("taip", "TAIP");
		o.rmempty  = false;
        
        o = s.option(form.Value, 'tid', _('NMEA station code or TAIP ID'));
		o.datatype    = 'or(length(0),length(4))';
		o.placeholder = '';
        o.rmempty = true;

		o = s.option(form.ListValue, 'tcp', _('TCP/UDP'), _('Use TCP connection to host or send UDP packets'));
        o.value("true",  "TCP");
        o.value("false", "UDP");
//		o.default  = o.enabled;
        o.rmempty  = false;

        o = s.option(form.Value, 'rate', _('Rate'), _('Optional rate limit in seconds'));
		o.datatype    = 'rate';
        o.datatype    = 'and(min(0),max(3600))';
		o.placeholder = 1;
        
		return m.render();
	}
});
