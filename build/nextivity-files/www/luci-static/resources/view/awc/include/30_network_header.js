'use strict';
'require baseclass';

// Dummy to provide section header on the combined status page. 

return baseclass.extend({
	title: _('Networking'),
    id: 'networking',

    render: function() {
       var netstatus = E([]); // E('div', { 'class': 'network-status-table' });
       
       return netstatus;
    }
});
