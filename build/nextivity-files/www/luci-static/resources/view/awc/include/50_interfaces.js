'use strict';
'require baseclass';
'require fs';
'require network';
'require uci';


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
	title: _('Interfaces'),
    id: 'interfaces',

	load: function() {
		return Promise.all([
			fs.trimmed('/proc/sys/net/netfilter/nf_conntrack_count'),
			fs.trimmed('/proc/sys/net/netfilter/nf_conntrack_max'),
            network.getNetworks(),
            L.resolveDefault(uci.load('awc')),
            fs.trimmed('/sys/class/net/wwan0/qmi/bridge_ipv4'),
            fs.trimmed('/sys/class/net/wwan0/qmi/bridge_mac')      
		]);
	},

	render: function(data) {
		var ct_count  = +data[0],
		    ct_max    = +data[1],
            networks = data[2];
            
        var bridge_ip  = data[4];
        var bridge_mac = data[5];
        
        var interfacestatus = E('table', { 'class': 'table interfaces' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				E('th', { 'class': 'th', 'width': '8%' }, _('Type')),
				E('th', { 'class': 'th', 'width': '18%' }, _('MAC')),
				E('th', { 'class': 'th', 'width': '20%' }, _('RX')),
				E('th', { 'class': 'th', 'width': '20%' }, _('TX')),
                E('th', { 'class': 'th', 'width': '16%' }, _('IPv4')),
                E('th', { 'class': 'th', 'width': '18%' }, _('IPv6'))
			])
		]);    
            
        var interfaces = [];

        var bridge_mode = uci.get('awc', 'main', 'bridge_mode');
        var lastrx = "";
        var lasttx = "";
        
        for (var count = 0; count < networks.length; count++) {
			var ifc = networks[count];
            var name = ifc.getName();
            
            if (name == "loopback") continue;
            
            var dev = ifc.getL3Device() || ifc.getDevice();
            if (dev) {
                var type    = name.toUpperCase();
                var mac     = dev.getMAC();
                var rxbytes = lastrx = '%.2mB (%d %s)'.format(dev.getRXBytes(), dev.getRXPackets(), _('Pkts.'));
                var txbytes = lasttx = '%.2mB (%d %s)'.format(dev.getTXBytes(), dev.getTXPackets(), _('Pkts.'));
                
                var addrsv4 = ifc.getIPAddrs();
                var addrsv6 = ifc.getIP6Addrs();
                 
                var ipv4    = (addrsv4 && addrsv4[0]) ? addrsv4[0] : '';
                var ipv6    = (addrsv6 && addrsv6[0]) ? addrsv6[0] : '';
                
                interfaces.push(
                    [type, mac, rxbytes, txbytes, ipv4, ipv6]
                );
            }    
        }   
        
        // Generate Passthrough mode interface display representing the LAN side computer connection 
        if (bridge_mode == "on") {
            var type = "Passthrough";
            var mac  = bridge_mac.toUpperCase();
            var rxbytes = lastrx;
            var txbytes = lasttx;
            
            var ipv4 = bridge_ip;
            var ipv6 = "";
            
            interfaces.push(
              [type, mac, rxbytes, txbytes, ipv4, ipv6]
            );
        }
                  
        cbi_update_table(interfacestatus, interfaces);      
                  
		var fields = [
			_('Active Connections'), ct_max ? ct_count : null
		];

		var ctstatus = E('table', { 'class': 'table' });

		for (var i = 0; i < fields.length; i += 2) {
			ctstatus.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [
					(fields[i + 1] != null) ? progressbar(fields[i + 1], ct_max) : '?'
				])
			]));
		}
                 
		return E([
			interfacestatus,
			ctstatus
		]);
	}
});
