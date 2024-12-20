'use strict';
'require baseclass';

// Dummy to provide section header on the combined status page. 

return baseclass.extend({
	title: _('Admin Tools'),
    id: 'admin-tools',

    render: function() {
       var adminstatus = E([]); // E('div', { 'class': 'admin-status-table' });
       
       return adminstatus;
    }
});
