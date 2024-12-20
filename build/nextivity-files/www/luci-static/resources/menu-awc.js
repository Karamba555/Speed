'use strict';
'require baseclass';
'require ui';
'require uci';
'require rpc';

var awc_config;
var count = 0;
var bypass = undefined;

return baseclass.extend({
	__init__: function() {
        this.load().then(L.bind(this.render, this));
	},

    load: function() {
        var session = rpc.getSessionID();
        var validSession = (session != undefined) && !session.startsWith("00000000");

        //console.log("Session: " + session + " " + validSession);

        // If the config lookup is done before we have a session, then it will still fail
        // on first attempt until reload. This avoids that.

        return Promise.all([
            //L.resolveDefault(L.uci.load('awc_cloud')),
            ui.menu.load(),
            validSession //,
            //L.uci.load('awc_cloud')
            //L.resolveDefault(validSession ? L.uci.load('awc_cloud') : null),
        ]).catch(function(exception) {
            console.log("Error during load");
        })
    },

	render: function(values) {
		var node = values[0],
		    url = '';
        var validSession = values[1];
        //awc_config = values[2];

/*        if (!validSession) {
            L.uci.unload('awc_cloud');
        }*/

       /* if (validSession && awc_config == undefined) { 
            console.log("*** Failed to correctly load awc_cloud configuration");
            //location.reload();
        }*/
        
        /*if (validSession) {
            console.log("awc_cloud " + L.uci.get('awc_cloud', 'cloud', 'eula')); 
        }*/
            
		this.renderModeMenu(node);

		if (L.env.dispatchpath.length >= 3) {
			for (var i = 0; i < 3 && node && node.children; i++) {
				node = node.children[L.env.dispatchpath[i]];
				url = url + (url ? '/' : '') + L.env.dispatchpath[i];
			}

			if (node)
				this.renderTabMenu(node, url);
		}		
		var sidebar = document.querySelector('#menubar > .navigation');

        if (sidebar) {
			sidebar.addEventListener('click', ui.createHandlerFn(this, 'handleSidebarToggle'));
        } else {
            console.log("Cannot locate sidebar");
        }
 
        if (validSession) {
             try {
               L.fs.read('/tmp/nextivity/eula_bypass').then(function(data) { 
                   bypass = data;
                   //console.log("bypass: " + bypass);
               }).catch(function(exception) {
                   //console.log("Exception here"); 
               });   
                  // bypass = data[0]; }));
             } catch(exception) {
                 //console.error(exception);
             }

             L.uci.load("awc_cloud").then(L.bind(this.renderEula, this));
        }
	},
    
    handleAcceptEula: async function(ev) {
        ui.hideModal();
        
        var form = E('form', {
            method: 'post',
            action: L.env.cgi_base + '/cgi-userdetails',
            enctype: 'application/x-www-form-urlencoded'
        }, 
            [
                E('input', { type: 'hidden', name: 'sessionid', value: rpc.getSessionID() }),
                E('input', { type: 'hidden', name: 'first',     value: document.getElementById("reg-first").value }),
                E('input', { type: 'hidden', name: 'last',      value: document.getElementById("reg-last").value }),
                E('input', { type: 'hidden', name: 'company',   value: document.getElementById("reg-company").value }),
                E('input', { type: 'hidden', name: 'phone',     value: document.getElementById("reg-phone").value }),
                E('input', { type: 'hidden', name: 'email',     value: document.getElementById("reg-email").value })
            ]
        );

		ev.currentTarget.parentNode.appendChild(form);

        async function submitFormWithRetry(form, retries = 3) {
            for (let attempt = 1; attempt <= retries; attempt++) {
                try {
                    const response = await fetch(form.action, {
                        method: form.method,
                        headers: {
                            'Content-Type': form.enctype
                        },
                        body: new URLSearchParams(new FormData(form))
                    });
    
                    if (response.ok) {
                        console.log('Form submitted successfully.');
                        break;
                    } else {
                        console.warn(`Attempt ${attempt} failed with status ${response.status}.`);
                    }
                } catch (error) {
                    console.error(`Attempt ${attempt} encountered an error: ${error}`);
                }
    
                if (attempt < retries) {
                    console.log('Retrying...');
                } else {
                    console.error('All retries failed.');
                }
            }
        }
    
        // Call the function to submit with retry
        await submitFormWithRetry(form);
    
        // Clean up the form
        form.parentNode.removeChild(form);

        window.location.reload();
        
    },
    
    handleDeclineEula: function() {
        // Return to login
        ui.hideModal();
        window.location.href = "/cgi-bin/luci/admin/logout";
    },

    textEntryChange : function(ev) {
//        console.log("textEntryChange");
      
        var valid = true;
        
        var firstName = document.getElementById("reg-first");
        if (!firstName || firstName.value.length == 0) {
            valid = false;
        }
        
        var lastName = document.getElementById("reg-last");
        if (!lastName || lastName.value.length == 0) {
            valid = false;
        }
        
        var email = document.getElementById("reg-email");
        if (!email || email.value.length == 0 || !email.value.includes("@")) {
            valid = false;
        }

        // Bypass value makes all valid
        var phone = document.getElementById("reg-phone");
        
        if (phone) {
          var number = phone.value.trim();
           //console.log("textEntryChange: " + number);
           
           var numberStrip = "";
           
           for (var n = 0; n < number.length; n++) {
             var c = number.charAt(n);
             
             if (n == 0 && c == '1') continue; // Leading '1'
             if (c == '.' || c == '-') continue;
                        
              numberStrip += c;
           }

           if (numberStrip == "8588592942") {
             valid = true;
           }
        }
        
//        console.log("Valid: " + valid);
        
        var accept = document.getElementById("eula-accept");
        
        if (accept) {
          if (accept.disabled == valid) accept.disabled = !valid
        }
    },
    
    addEntry: function(rows, text, id) {
        var textEntry = E('td', { 'class': 'td left', 'style': 'width:300px' }, [
						E('input', {
                            'id': id,
                            'change' : ui.createHandlerFn(this, this.textEntryChange),
							'type': 'text',
							'value': ''
						})
                    ]);
        
        var row = E('tr', { 'class': 'tr' }, [
                    E('td', { 'class': 'td left',  'style': 'width:200px' }, [
                        text, 
                    ]), 
                    textEntry 
                ]);
        
        rows.push(row);
    },
    
    renderEula: function(config) {
        var accept = undefined;
       
  //      console.log("renderEula: " + config);
        
        if (bypass != undefined && bypass.startsWith("yes")) {
           console.log("EULA bypass invoked");
           return;
        }
        
        try { 
                accept = L.uci.get('awc_cloud', 'cloud', 'eula'); 
                
        } catch (exception) {
            console.log("Exception during EULA status fetch");
            return; 
        }
        
        //console.log("Accept: " + accept);

        // Undefined would mean a lookup problem 
        if (accept == undefined) {
           console.log("*** Failed to determine awc_cloud config values");   
        }

        if (accept == "yes") {
          var passPage =  "/cgi-bin/luci/admin/system/admin/password";

          if (!window.location.href.includes(passPage)) {
            window.location.href = passPage;
          }

          return;

        } else if (accept == "yes_pass") {
          return;

        }
            
        var popup = []; 
        var source = "/luci-static/resources/Nextivity-EULA.html";
       
        popup.push(E('iframe', 
                    { 
                      'src': source,
                      'height': 500
                    }
        ));
        

        var rows = [];
        
        this.addEntry(rows, "First Name", "reg-first");
        this.addEntry(rows, "Last Name", "reg-last");
        this.addEntry(rows, "Company (optional)", "reg-company");
        this.addEntry(rows, "Phone (optional)", "reg-phone");
        this.addEntry(rows, "E-Mail", "reg-email");
        
        var table = E('table', { 'class': 'table', 'style': 'width:50%' }, rows);
        
        popup.push(table);

        var acceptButton = E('button', {
                        'class': 'btn cbi-button-action important',
                        'click': ui.createHandlerFn(this, 'handleAcceptEula'),
                        'id': 'eula-accept',
                        'disabled': true
                    }, [ _('Accept') ] );
      
        var declineButton = E('button', {
                        'class': 'btn cbi-button-action important',
                        'click': ui.createHandlerFn(this, 'handleDeclineEula'),
                        'id': 'eula-decline'
                    }, [ _('Decline') ] );
       
       
        popup.push(E('div', { 'class': 'right' }, [
            acceptButton, ' ', declineButton ]));
       
        ui.showModal(_("End User Licence Agreement"), popup);

        var phone = document.getElementById("reg-phone");
          
//        console.log("Adding input handler");
        phone.addEventListener('input', L.bind(this.textEntryChange, this));
    },
    
	handleMenuExpand: function(ev) {
		var a = ev.target, ul1 = a.parentNode.parentNode, ul2 = a.nextElementSibling;

		document.querySelectorAll('ul.mainmenu.l1 > li.active').forEach(function(li) {
			if (li !== a.parentNode)
				li.classList.remove('active');
		});

		if (!ul2)
			return;

		if (ul2.parentNode.offsetLeft + ul2.offsetWidth <= ul1.offsetLeft + ul1.offsetWidth)
			ul2.classList.add('align-left');

		ul1.classList.add('active');
		a.parentNode.classList.add('active');
		a.blur();

		ev.preventDefault();
		ev.stopPropagation();
	},

	renderMainMenu: function(tree, url, level) {
		var l = (level || 0) + 1,
		    ul = E('ul', { 'class': 'mainmenu l%d'.format(l) }),
		    children = ui.menu.getChildren(tree);

		if (children.length == 0 || l > 2)
			return E([]);

        var path = L.env.dispatchpath[l];

//        console.log("level: " + l + " path: " + path);

        if ((l == 1) && (path == "awc")) {
            var children = [

                { title: "Device",                name : "device",     type: "section" },
                { title: "MegaFi Status", name : "system-status",  },
                { title: "AW12 Status",           name : "aw12-status" },
                { title: "Networking",            name : "networking", type: "section" },
                //{ title: "Active DHCP Leases",    name : "dhcp4"       },
                //{ title: "Active DHCPv6 Leases",  name : "dhcp6"       },
                { title: "DHCP Leases",           name : "dhcp"       },
                { title: "Interfaces",            name : "interfaces"  },
                //{ title: "Admin Tools",           name : "admin-tools", type: "section" },
                { title: "System Settings",       name : "admin-system-settings", type: "section"},
                { title: "Logout",                name : "logout", type: "section" },
                
                // This will only work as written with one sublevel 
                { title: "Overview",              name : "maincontainer",   type: "heading" }
            ];
            
            var subul = E('ul', { 'class': 'mainmenu l%d'.format(l + 1) });
          
            for (var i = 0; i < children.length; i++) {
                var style = '';
                var type = children[i].type ? children[i].type : "";
                var name = children[i].name;
 
                var isActive = true;
                var activeClass = 'mainmenu-item-%s%s'.format(name, isActive ? ' selected' : '');
              
                if (type == "heading") {
                    //style = "font-size: 1.5em; padding: 20px 0px";
                } else if (type == "section") {
                //    style = "font-size: 1.2em padding: 10px 0px ";
                } else {
                    style = "text-indent: 20px"; 
                }
                
                var href;
                
                if (name == "logout") {
                    href = "/cgi-bin/luci/admin/logout";
                } else {
                    href = "#" + name;  
                }
                          
                if (type == "heading") {
                     ul.appendChild(E('li', { 'class': activeClass,  'style' : style }, [
                        E('a', {
                            'href': href
                        }, [ _(children[i].title) ]), subul
                    ])); 
                     
                } else {
                    subul.appendChild(E('li', { 'class': activeClass,  'style' : style }, [
                        E('a', {
                            'href': href
                        }, [ _(children[i].title) ]) /*,*/
                    ]));
                }
            }
            
        } else {    
            
            for (var i = 0; i < children.length; i++) {
                var name = children[i].name;

                // From Status
 /*               if (name == "iptables")  continue;
                if (name == "routes")    continue;
                if (name == "processes") continue;
                if (name == "dmesg")     continue;
                        
                // From System
                if (name == "startup")   continue;
                if (name == "crontab")   continue;*/
                if (name == "mounts")    continue;
                if (name == "leds")      continue;

                // From Network
/*                if (name == "switch")   continue;
                if (name == "dhcp")     continue;
                if (name == "hosts")    continue;
                if (name == "firewall") continue;*/

                var title   = children[i].title;
                var submenu = true;
                var style   = '';

                if ((name == "awc") && (l == 1)) {
                    title = "Overview";
                    name = "awc/overview";
                    submenu = false;
                    style = "font-size: 1.0em; padding: 20px 0px";
                }

                var isActive = (L.env.dispatchpath[l] == name),
                    isReadonly = children[i].readonly,
                    activeClass = 'mainmenu-item-%s%s'.format(name, isActive ? ' selected' : '');

                ul.appendChild(E('li', { 'class': activeClass, 'style': style }, [
                    E('a', {
                        'href': L.url(url, name),
                        'click': (l == 1) ? ui.createHandlerFn(this, 'handleMenuExpand') : null
                    }, [ _(title) ]),
                    submenu ? this.renderMainMenu(children[i], url + '/' + name, l) : ""
                ]));
            }
        }

		if (l == 1)
			document.querySelector('#mainmenu').appendChild(E('div', [ ul ]));

		return ul;
	},

	renderModeMenu: function(tree) {
		var menu = document.querySelector('#modemenu'),
		    children = ui.menu.getChildren(tree);

		for (var i = 0; i < children.length; i++) {
			var isActive = (L.env.requestpath.length ? children[i].name == L.env.requestpath[0] : i == 0);

			if (i > 0)
				menu.appendChild(E([], ['\u00a0|\u00a0']));

			menu.appendChild(E('div', { 'class': isActive ? 'active' : null }, [
				E('a', { 'href': L.url(children[i].name) }, [ _(children[i].title) ])
			]));

			if (isActive)
				this.renderMainMenu(children[i], children[i].name);
		}

		if (menu.children.length > 1)
			menu.style.display = '';
	},

	renderTabMenu: function(tree, url, level) {
		var container = document.querySelector('#tabmenu'),
		    l = (level || 0) + 1,
		    ul = E('ul', { 'class': 'cbi-tabmenu' }),
		    children = ui.menu.getChildren(tree),
		    activeNode = null;

		if (children.length == 0)
			return E([]);

		for (var i = 0; i < children.length; i++) {
			var isActive = (L.env.dispatchpath[l + 2] == children[i].name),
			    activeClass = isActive ? ' cbi-tab' : '',
			    className = 'tabmenu-item-%s %s'.format(children[i].name, activeClass);

			ul.appendChild(E('li', { 'class': className }, [
				E('a', { 'href': L.url(url, children[i].name) }, [ _(children[i].title) ] )
			]));

			if (isActive)
				activeNode = children[i];
		}

		container.appendChild(ul);
		container.style.display = '';

		if (activeNode)
			container.appendChild(this.renderTabMenu(activeNode, url + '/' + activeNode.name, l));

		return ul;
	},

	handleSidebarToggle: function(ev) {
		var btn = ev.currentTarget,
		    bar = document.querySelector('#mainmenu');

		if (btn.classList.contains('active')) {
			btn.classList.remove('active');
			bar.classList.remove('active');
		}
		else {
			btn.classList.add('active');
			bar.classList.add('active');
		}
	}
});
