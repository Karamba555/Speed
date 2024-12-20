'use strict';
'require baseclass';
'require form';
'require fs';
'require rpc';
'require uci';
'require network';
'require ui';
'require request';

var mapdata = { actions: {}, config: {} };

var storage_size = 34 * 1024 * 1024 ; // TODO fill me in

var push_val,          push_val_orig;
var push_period_val,   push_period_val_orig; 
var poll_period_val,   poll_period_val_orig;
var cycle_val,         cycle_val_orig;
var mcu_val,           mcu_val_orig;
var shutdown_val,      shutdown_val_orig;
var apn_val,           apn_val_orig;
var lanip_val,         lanip_val_orig;
var auto_firmware_val, auto_firmware_val_orig;
var auto_config_val,   auto_config_val_orig;
var wan_conn_val,      wan_conn_val_orig;
var lan_wan_val,       lan_wan_val_orig;

var delayDropdown;


var callReboot = rpc.declare({
	object: 'system',
	method: 'reboot',
	expect: { result: 0 }
});


var callSystemValidateFirmwareImage = rpc.declare({
        object: 'system',
        method: 'validate_firmware_image',
        params: [ 'path' ],
        expect: { '': { valid: false, forcable: true } }
});


return baseclass.extend({
	title: _('System Settings'),
    id: 'admin-system-settings',

	load: function() {
		return Promise.all([
            L.resolveDefault(uci.load('network')),
            L.resolveDefault(uci.load('awc')),
            L.resolveDefault(uci.load('awc_cloud')),
            fs.lines('/etc/nextivity_build_info')
        ]);
	},
    
    handleUpdateFirmware: function(/*storage_size, */ev) {
		return ui.uploadFile('/tmp/firmware.bin', ev.target.firstChild)
			.then(L.bind(function(btn, reply) {
				btn.firstChild.data = _('Checking image…');

				ui.showModal(_('Checking image…'), [
					E('span', { 'class': 'spinning' }, _('Verifying the uploaded image file.'))
				]);

				return callSystemValidateFirmwareImage('/tmp/firmware.bin')
					.then(function(res) { return [ reply, res ]; });
			}, this, ev.target))
			.then(L.bind(function(btn, reply) {
				return fs.exec('/sbin/sysupgrade', [ '--test', '/tmp/firmware.bin' ])
					.then(function(res) { reply.push(res); return reply; });
			}, this, ev.target))
			.then(L.bind(function(btn, res) {
				var keep = E('input', { type: 'checkbox' }),
				    force = E('input', { type: 'checkbox' }),
				    is_valid = res[1].valid,
				    is_forceable = res[1].forceable,
				    allow_backup = res[1].allow_backup,
				    is_too_big = (storage_size > 0 && res[0].size > storage_size),
				    body = [];

				body.push(E('p', _("The flash image was uploaded. Below is the checksum and file size listed, compare them with the original file to ensure data integrity. <br /> Click 'Continue' below to start the flash procedure.")));
				body.push(E('ul', {}, [
					res[0].size ? E('li', {}, '%s: %1024.2mB'.format(_('Size'), res[0].size)) : '',
					res[0].checksum ? E('li', {}, '%s: %s'.format(_('MD5'), res[0].checksum)) : '',
					res[0].sha256sum ? E('li', {}, '%s: %s'.format(_('SHA256'), res[0].sha256sum)) : ''
				]));

				body.push(E('p', {}, E('label', { 'class': 'btn' }, [
					keep, ' ', _('Keep settings and retain the current configuration')
				])));

				if (!is_valid || is_too_big)
					body.push(E('hr'));

				if (is_too_big)
					body.push(E('p', { 'class': 'alert-message' }, [
						_('It appears that you are trying to flash an image that does not fit into the flash memory, please verify the image file!')
					]));

				if (!is_valid)
					body.push(E('p', { 'class': 'alert-message' }, [
						res[2].stderr ? res[2].stderr : '',
						res[2].stderr ? E('br') : '',
						res[2].stderr ? E('br') : '',
						_('The uploaded image file does not contain a supported format. Make sure that you choose the generic image format for your platform.')
					]));

				if (!allow_backup)
					body.push(E('p', { 'class': 'alert-message' }, [
						_('The uploaded firmware does not allow keeping current configuration.')
					]));

				if (allow_backup)
					keep.checked = true;
				else
					keep.disabled = true;


				if ((!is_valid || is_too_big) && is_forceable)
					body.push(E('p', { 'class': 'alert-message danger' }, [
						force, ' ', _('Force upgrade: Select \'Force upgrade\' to flash the image even if the image format check fails. Use only if you are sure that the firmware is correct and meant for your device!')
					]));

				var cntbtn = E('button', {
					'class': 'btn cbi-button-action important',
					'click': ui.createHandlerFn(this, 'handleSysupgradeConfirm', btn, keep, force),
					'disabled': (!is_valid || is_too_big) ? true : null
				}, [ _('Continue') ]);

				body.push(E('div', { 'class': 'right' }, [
					E('button', {
						'class': 'btn',
						'click': ui.createHandlerFn(this, function(ev) {
							return fs.remove('/tmp/firmware.bin').finally(ui.hideModal);
						})
					}, [ _('Cancel') ]), ' ', cntbtn
				]));

				force.addEventListener('change', function(ev) {
					cntbtn.disabled = !ev.target.checked;
				});

				ui.showModal(_('Flash image?'), body);
			}, this, ev.target))
			.catch(function(e) { ui.addNotification(null, E('p', e.message)) })
			.finally(L.bind(function(btn) {
				btn.firstChild.data = _('Flash image...');
			}, this, ev.target));
        
    },
    
    
    handleSysupgradeConfirm: function(btn, keep, force, ev) {
        btn.firstChild.data = _('Flashing…');

        ui.showModal(_('Flashing…'), [
                E('p', { 'class': 'spinning' }, _('The system is flashing now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings.'))
                ]);

        var opts = [];

        if (!keep.checked)
            opts.push('-n');

        if (force.checked)
            opts.push('--force');

        opts.push('/tmp/firmware.bin');

        /* Currently the sysupgrade rpc call will not return, hence no promise handling */
        fs.exec('/sbin/sysupgrade', opts);

        if (keep.checked)
            ui.awaitReconnect(window.location.host);
        else
            ui.awaitReconnect(keep.checked ? lanip_val : '192.168.113.1', 'openwrt.lan');
    },

    handleVersionUpdate: function(ev) {
        //console.log("Handle version update");
    },
    
    handleSaveFile : function(ev) {
        var form = E('form', {
			method: 'post',
			action: L.env.cgi_base + '/cgi-backup',
			enctype: 'application/x-www-form-urlencoded'
		}, E('input', { type: 'hidden', name: 'sessionid', value: rpc.getSessionID() }));

		ev.currentTarget.parentNode.appendChild(form);

		form.submit();
		form.parentNode.removeChild(form);
	},
        
    
    handleLoadFile : function(ev) {
        return ui.uploadFile('/tmp/backup.tar.gz', ev.target)
			.then(L.bind(function(btn, res) {
				btn.firstChild.data = _('Checking archive…');
				return fs.exec('/bin/tar', [ '-tzf', '/tmp/backup.tar.gz' ]);
			}, this, ev.target))
			.then(L.bind(function(btn, res) {
				if (res.code != 0) {
					ui.addNotification(null, E('p', _('The uploaded backup archive is not readable')));
					return fs.remove('/tmp/backup.tar.gz');
				}

				ui.showModal(_('Apply backup?'), [
					E('p', _('The uploaded backup archive appears to be valid and contains the files listed below. Press "Continue" to restore the backup and reboot, or "Cancel" to abort the operation.')),
					E('pre', {}, [ res.stdout ]),
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.createHandlerFn(this, function(ev) {
								return fs.remove('/tmp/backup.tar.gz').finally(ui.hideModal);
							})
						}, [ _('Cancel') ]), ' ',
						E('button', {
							'class': 'btn cbi-button-action important',
							'click': ui.createHandlerFn(this, 'handleRestoreConfirm', btn)
						}, [ _('Continue') ])
					])
				]);
			}, this, ev.target))
			.catch(function(e) { ui.addNotification(null, E('p', e.message)) })
			.finally(L.bind(function(btn, input) {
				btn.firstChild.data = _('Upload archive...');
			}, this, ev.target));
    },
    
    handleRestoreConfirm: function(btn, ev) {
        return fs.exec('/sbin/sysupgrade', ['--restore-backup', '/tmp/backup.tar.gz']).then(L.bind(function(btn, res) {
            if (res.code != 0) {
                ui.addNotification(null, [E('p', _('The restore command failed with code %d').format(res.code)), res.stderr ? E('pre', {}, [res.stderr]) : '']);
                L.raise('Error', 'Unpack failed');
            }
            btn.firstChild.data = _('Rebooting…');
            return fs.exec('/sbin/reboot');
        }, this, ev.target)).then(L.bind(function(res) {
            if (res.code != 0) {
                ui.addNotification(null, E('p', _('The reboot command failed with code %d').format(res.code)));
                L.raise('Error', 'Reboot failed');
            }
            ui.showModal(_('Rebooting…'), [E('p', {
                'class': 'spinning'
            }, _('The system is rebooting now. If the restored configuration changed the current LAN IP address, you might need to reconnect manually.'))]);
            ui.awaitReconnect(window.location.host, '192.168.113.1', 'openwrt.lan');
        }, this)).catch(function(e) {
            ui.addNotification(null, E('p', e.message))
        }).finally(function() {
            btn.firstChild.data = _('Upload archive...')
        });
    },

    handleChangePassword : function() {
        // Redirect for now, handlle fully later
        window.location.href="/cgi-bin/luci/admin/system/admin/password";
    },
    
    handleFactoryDefaults : function() {
        if (!confirm(_('Do you really want to return settings to factory defaults?')))
			return;

		ui.showModal(_('Erasing...'), [
                        E('p', { 'class': 'spinning' }, _('The system is erasing the configuration partition now and will reboot itself when finished.'))
                ]);

		/* Will not return, hence no promise handling */
		fs.exec('/sbin/firstboot', [ '-r', '-y' ]);

		ui.awaitReconnect('192.168.113.1', 'openwrt.lan');		
    },
    
    handleExpertConfiguration : function() {
        
        if (!confirm(_('The Expert configuration section contains items that may render the MegaFi inoperative and require a factory reset. Please confirm you want to proceed.')))
            return;
        
        window.location.href="/cgi-bin/luci/admin/status";
    },
    
    handleReboot : function(ev) {
        if (!confirm(_('Do you really want to reboot now?')))
			return;
        
        return callReboot().then(function(res) {
			if (res != 0) {
				L.ui.addNotification(null, E('p', _('The reboot command failed with code %d').format(res)));
				L.raise('Error', 'Reboot failed');
			}

			L.ui.showModal(_('Rebooting…'), [
				E('p', { 'class': 'spinning' }, _('Waiting for device...'))
			]);

			window.setTimeout(function() {
				L.ui.showModal(_('Rebooting…'), [
					E('p', { 'class': 'spinning alert-message warning' },
						_('Device unreachable! Still waiting for device...'))
				]);
			}, 150000);

			L.ui.awaitReconnect();
		})
		.catch(function(e) { L.ui.addNotification(null, E('p', e.message)) });
    },
    
    handleReportToCloud: function(m, ev, value) {
        uci.set('awc', 'status_gather', 'logging_push', value); 
    },
    
    fieldsAppendStatus: function(table, fields) {
        for (var i = 0; i < fields.length; i += 2) {
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [ (fields[i + 1] != null) ? fields[i + 1] : '?' ])
			]));
		}
    },
    
    fieldsAppendButton: function(table, fields) {
        for (var i = 0; i < fields.length; i += 3) {
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
                E('td', { 'class': 'td left', 'width': '33%' }, 
                    E('button', {
                        'class': 'cbi-button cbi-button-apply',
                        'click': L.bind(fields[i + 2], this),
                    }, [ _(fields[i + 1])])
                ),
                E('td', { 'class': 'td left top', 'width': '25%' }, [])
			]));
		}
    },
    
    fieldsAppendDropdown: function(table, fields, enabled, custom, datatype, buttonText, buttonAction) {
       var m, s, o, ss;
        
        m = new form.JSONMap(mapdata);
        
        s = m.section(form.NamedSection, 'actions');
		o = s.option(form.SectionValue, 'actions', form.NamedSection, 'actions', 'actions');
		ss = o.subsection;
  
        //var dropdown = ss.option(form.ListValue, fields[0]);
        var dropdown = ss.option(form.MultiValue, fields[0]);
        
        for (var item = 0; item < fields[2].length; item++) {
            dropdown.value(fields[2][item][0], fields[2][item][1]);
        }    
        
        dropdown.default     = fields[3];
        dropdown.onchange    = L.bind(fields[4], this);
        dropdown.readonly    = (fields[0] == "shutdown") ? false : !enabled;
        dropdown.create      = custom;
        dropdown.multiple    = false;
        dropdown.placeholder = false;
        dropdown.datatype    = datatype;
        
        
        var cell = E('td', { 'class': 'td left top' }, []);

        var buttonCol;
        
        if (buttonText) {
            buttonCol = E('button', {
					'class': 'cbi-button cbi-button-apply',
                    'id' : 'flash-button',              // Only one 3rd column button for now, disabled to start
                    'disabled' : true, 
					'click': L.bind(buttonAction, this),
                }, [ _(buttonText)]);
           
        } else {
          buttonCol =  E('td', { 'class': 'td left', 'width': '25%' }, [ " " ]); 
        }
        
        var display = (fields[0] == "shutdown" && !enabled) ? "hidden" : null; 
        
        table.appendChild(E('tr', { 'class': 'tr', 'id': 'tr-' + fields[0], 'display': display }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[1] ]), 
                cell,
                buttonCol
            ]));

        m.render().then(function(node) {
            cell.appendChild(node); 
        });        
        
        return dropdown;
    },
    
    /**
     * Clear all values then re-set to the changed values. 
     * This allow us to do the settings on the admin screen more
     * dyanmically, without stacking up uci changes.
     */ 
    
    setUciValues: function() {
       
        request.request(L.url('admin/uci/revert'), {
                method: 'post',
                query: { sid: L.env.sessionid, token: L.env.token }
            }).then(function(r) {
                if (r.status === 200) {
                    document.dispatchEvent(new CustomEvent('uci-reverted'));
                    ui.changes.setIndicator(0);
                    
                    var doSave = false;    
            
                    if (push_val != push_val_orig) {
                        uci.set('awc', 'status_gather', 'logging_push', push_val);
                        doSave = true;
                    }
                   
                    if (push_period_val != push_period_val_orig) {
                        uci.set('awc', 'status_gather', 'period', push_period_val);
                        doSave = true;
                    }                    
                    
                    if (poll_period_val != poll_period_val_orig) {
                        uci.set('awc', 'status_gather', 'poll', poll_period_val);
                        doSave = true;
                    }
                   
                    if (cycle_val != cycle_val_orig) {
                        uci.set('awc', 'main', 'cycle_ethernet', cycle_val);
                        doSave = true;
                    }
        
                    if (shutdown_val != shutdown_val_orig) {
                        //console.log("shutdown_val: " + shutdown_val);
                        uci.set('awc', 'mcu', 'shutdown_period', shutdown_val);
                        doSave = true;
                    }

                    if (apn_val != apn_val_orig) {
                        uci.set('network', 'wwan', 'apn', apn_val);
                        doSave = true;
                    }

                    if (lanip_val != lanip_val_orig) {
                        uci.set('network', 'lan', 'ipaddr', lanip_val);
                        doSave = true;
                    }
                    
                    if (wan_conn_val != wan_conn_val_orig) {
                        uci.set('network', 'wan', 'auto', wan_conn_val);
                        doSave = true;
                    }

                    if (auto_firmware_val != auto_firmware_val_orig) {
                        uci.set('awc_cloud', 'cloud', 'auto_firmware', auto_firmware_val);
                        doSave = true;
                    }

                    if (auto_config_val != auto_config_val_orig) {
                        uci.set('awc_cloud', 'cloud', 'auto_config', auto_config_val);
                        doSave = true;
                    }

                    if (lan_wan_val != lan_wan_val_orig) {
                        if (lan_wan_val == 'lan') {
                            // switch from wan to lan for port LAN2 
                            uci.set('network', 'wan', 'device', '')
                            uci.set('network', 'wan', 'proto', '')
                            uci.set('network', '@device[0]', 'ports', 'lan1 wan')
                        } else {
                            // switch from lan to wan for port LAN2
                            uci.set('network', 'wan', 'device', 'wan')
                            uci.set('network', 'wan', 'proto', 'dhcp')
                            uci.set('network', '@device[0]', 'ports', 'lan1')

                        }
                        doSave = true;
                    }
                    
                    if (doSave) uci.save();
                }
            }); 
    },
    
    
    handleAPN: function(m, ev, value) {
        //console.log("Handle APN: " + value);
        
        apn_val = value;
        this.setUciValues();
    }, 
    
    handlePush: function(m, ev, value) {
        push_val = value;
        this.setUciValues();
    },
    
    handlePushPeriod: function(m, ev, value) {
        push_period_val = value;
        this.setUciValues();
    },
        
    handlePollPeriod: function(m, ev, value) {
        poll_period_val = value;
        this.setUciValues();
    },
     
    handleCycle: function(m, ev, value) {
        cycle_val = value;
        this.setUciValues();
    },
     
    handleLanIP: function(m, ev, value) {
        //console.log("Handle LAN: " + value);
        
        lanip_val = value;
        
        if (lanip_val != lanip_val_orig) {
           alert("Changing the LAN IP address may render the system temporarily unreachable. You may need to manually reconfigure your host system's interface and visit the new address manually.");   
        }
        
        this.setUciValues();
    },
    
    handleWanConn: function(m, ev, value) {
        wan_conn_val = value;
        this.setUciValues();
    },

    handleLanWan: function(m, ev, value) {
        if (value == 'wan') {
            lan_wan_val = 'wan';
        } else {
            lan_wan_val = 'lan';
        }
        this.setUciValues();
    },
     

    handleAutoUpdateFirmware:  function(m, ev, value) {
        //console.log("Handle LAN: " + value);

        auto_firmware_val = value;
        this.setUciValues();
    },

    handleAutoUpdateConfig:  function(m, ev, value) {
        //console.log("Handle LAN: " + value);

        auto_config_val = value;
        this.setUciValues();
    },
        
    setShutdownEnabled: function(enabled) {
       var dropdown = document.getElementById("tr-shutdown");
        
       if (dropdown) {
           console.log("shutdown enabled: " + enabled);
           
           if (enabled) {
             dropdown.style.removeProperty("display");   
           } else {
             dropdown.style.display = "none";
           }
           
       } else {
           console.log("Unable to locate shutdown dropdown");  
       }
    },
    
    handleShutdown: function(m, ev, value) {
        shutdown_val = value;
        this.setUciValues();
    },
        
    handleMCU: function(m, ev, value) {
        mcu_val = value;
        
        var button = document.getElementById("flash-button"); 
        
        if (button) {
            button.disabled = (mcu_val == mcu_val_orig);  
        } else {
            console.log("Unable to locate flash-button");
        }
        
        this.setShutdownEnabled(mcu_val == "vehicle");  
    },
    
    handleMCUFlash: function() {
        //console.log("Handle MCU flash: " + mcu_val);
        
        if (!confirm(_('Flashing the MCU takes about 4 minutes.\nThe MegaFi will reboot after. You will then need to additionally power cycle the MegaFi to activate the new MCU code'))) {
          return;
        }
        
		ui.showModal(_('Flashing MCU…'), [
					E('p', { 'class': 'spinning' }, _('Flashing MCU. DO NOT POWER CYCLE the MegaFi before the flashing process completes!'))
				]);

		// Flashing takes about 3 1/2 minutes
		var timeout = (4 * 60);
		
		setTimeout(function() {
		  //console.log("flash completed");	
		  ui.hideModal();
          
          ui.showModal(_('Waiting for system restart…'), [
					E('p', { 'class': 'spinning' }, _('The system is rebooting now.'))
				]);
          
          var ip = L.uci.get('network', 'lan', 'ipaddr');

          if (ip == undefined) ip = lanip_val;
    
          ui.awaitReconnect(window.location.host, ip, 'openwrt.lan');
          
		}, (timeout * 1000));
		
		// This will throw an XHR exception for timeout. The process
		// is still running
		try { 
			//console.log("Calling script");
			fs.exec('/sbin/mcu-flash.sh', [ '-d' , mcu_val ]).catch(function(error) {
				
			   //console.log("Promise exception: " + error.message);	
			});
			//console.log("Script done");
		} catch (error) { 
		    //console.log("Exception caught: " + error);	
		}
    },
    
	render: function(data) {
        var build = data[3];
        var skin =  L.uci.get('awc', 'main', 'skin');
        
        var systemType = "";
        
        if (build && build[3]) {
          systemType = build[3].substring(14); // "Target board: megafi" 
        }
        
        var table = E('table', { 'class': 'table' });

        apn_val = apn_val_orig = uci.get('network', 'wwan', 'apn');
        
        var apn_options = [[ apn_val ]];
    
        if (apn_val != "firstnet-broadband") {
            apn_options.push(["firstnet-broadband"]);
        }    
        //apn_options.push(["change", _("Change...")]);
            
        var fields_apn = [
 /*           _('APN (Access Point Name)'), apn*/
              'apn',
                _('APN (Access Point Name)'),
                apn_options,
                apn_val,
                this.handleAPN,
                { create: true }
        ];
        
        lanip_val       = lanip_val_orig       = uci.get('network', 'lan', 'ipaddr');
        push_val        = push_val_orig        = uci.get('awc', 'status_gather', 'logging_push');
        push_period_val = push_period_val_orig = uci.get('awc', 'status_gather', 'period');
        poll_period_val = poll_period_val_orig = uci.get('awc', 'status_gather', 'poll');
        cycle_val       = cycle_val_orig       = uci.get('awc', 'main', 'cycle_ethernet');
        wan_conn_val    = wan_conn_val_orig    = uci.get('network', 'wan', 'auto');
        lan_wan_val     = lan_wan_val_orig     = uci.get('network', 'wan', 'device');

        if (lan_wan_val == undefined) {
            lan_wan_val = lan_wan_val_orig = 'lan';
        } else {
            lan_wan_val = lan_wan_val_orig = 'wan';
        }
        
        var fields_lanip = [ 
             'lan',
                _("LAN IP"),
                [
                    [lanip_val]
                ],
                lanip_val,
                this.handleLanIP,
                { create: true }
        ];
        
        var fields_wan_conn = [ 
             'wan',
                _("Enable Priority WAN connection"),
                [                    
                    ["1", "WAN On"],
                    ["0", "WAN Off"]
                ],
                wan_conn_val,
                this.handleWanConn,
        ];
                
        var fields_push = [
             'report',
                _("Report to Cloud"),
                [
                    ["on",  "Reporting On"],
                    ["off", "Reporting Off"]
                ],
                push_val,
                this.handlePush
        ];
        
        var fields_push_period = [
             'push_period',
                _("Report to Cloud Period (seconds)"),
                [
                    [push_period_val],
                ],
                push_period_val,
                this.handlePushPeriod,
                { create: true }
        ];       
        
        var fields_poll_period = [
             'poll_period',
                _("System Poll Period (seconds)"),
                [
                    [poll_period_val],
                ],
                poll_period_val,
                this.handlePollPeriod,
                { create: true }
        ]; 
        
        var fields_cycle = [
             'report',
                _("Cycle LAN upon WWAN IP change"),
                [
                    ["on",  "Cycle On"],
                    ["off", "Cycle Off"]
                ],
                cycle_val,
                this.handleCycle
        ];

        var fields_lan_wan = [
            'lanwan',
               _("Switch LAN2/WAN port mode"),
               [
                   ["wan",  "WAN"],
                   ["lan", "LAN"]
               ],
               lan_wan_val,
               this.handleLanWan
       ];
        
        var fields_buttons = [ 
            _("Update Firmware"),               "Upload Firmware", this.handleUpdateFirmware,
        
 // TODO: Full version polls for updates from the cloud, although doesn't have any function for applying manually                       
 //           _("Update Firmware"),               "Check for updates", this.handleUpdateFirmware,
            _("Backup Existing Configuration"), "Save to File",      this.handleSaveFile,
            _("Load Configuration from File"),  "Load File",         this.handleLoadFile,
            _("Change Password"),               "Change Password",   this.handleChangePassword,
            _("Factory Defaults"),              "Factory Defaults",  this.handleFactoryDefaults
        ];
        
        mcu_val = mcu_val_orig = uci.get('awc', 'mcu', 'mcu_application');
        
        // Disable in some cases
        var fields_poe = [
            'mcu',
            _("MCU Selection"), 
            [
                ["poe", "Power Over Ethernet"],
                ["vehicle", "Vehicle"] /*,
                ["custom", "Custom"]*/
            ],
            mcu_val,
            this.handleMCU
        ];

        shutdown_val = uci.get('awc', 'mcu', 'shutdown_period');

        if (shutdown_val == 12) {
            shutdown_val = 30;
            uci.set('awc', 'mcu', 'shutdown_period', shutdown_val);
            uci.save();
            this.setUciValues();
         }

        shutdown_val_orig = shutdown_val;
        
        var fields_shutdown = [
            'shutdown',
            _("Vehicle Shutdown Delay"),
            [
                ["30",    "30 Seconds"],
                ["900",   "15 Minutes"],
                ["3600",  "1 Hour"],
                ["7200",  "2 Hours"]
              //  ["10800", "3 Hours"],
              //  ["14400", "4 Hours"],
              //  ["28800", "8 Hours"]
            ],
            shutdown_val,
            this.handleShutdown
        ];

        auto_firmware_val = auto_firmware_val_orig = uci.get('awc_cloud', 'cloud', 'auto_firmware');

        //console.log("firmware " + auto_firmware_val);

        var fields_auto_firmware = [
               'update',
                _("Automatically Update Firmware"),
                [
                    ["on",  "Firmware Update On"],
                    ["off", "Firmware Update Off"]
                ],
                auto_firmware_val,
                this.handleAutoUpdateFirmware
        ];

        auto_config_val = auto_config_val_orig = uci.get('awc_cloud', 'cloud', 'auto_config');

        //console.log("config " + auto_config_val);

        var fields_auto_config = [
               'update',
                _("Automatically Update Configuration"),
                [
                    ["on",  "Config Update On"],
                    ["off", "Config Update Off"]
                ],
                auto_config_val,
                this.handleAutoUpdateConfig
        ];

        var fields_system = [
            _("Expert Configuration"), "Expert Configuration", this.handleExpertConfiguration,
            _("Reboot"),               "Reboot",               this.handleReboot
        ];
        
        var bridge_mode = uci.get('awc', 'main', 'bridge_mode');
        
        this.fieldsAppendDropdown(table, fields_apn,   true, true);
        this.fieldsAppendDropdown(table, fields_lanip, true, true, 'ipaddr');
        
        if (wan_conn_val != undefined) {
            this.fieldsAppendDropdown(table, fields_wan_conn, true, true, 'wan_conn');
        }
        
        if (bridge_mode != "on") {
            this.fieldsAppendDropdown(table, fields_cycle, true, false);  
            this.fieldsAppendDropdown(table, fields_lan_wan, true, false); 
        }
        else { 
            // disable switching of port LAN2 to lan mode in passthrough mode
            this.fieldsAppendDropdown(table, fields_lan_wan, false, false);
        }
        this.fieldsAppendDropdown(table, fields_push,        true, false);
//        this.fieldsAppendDropdown(table, fields_push_period, true, true, 'integer');
//        this.fieldsAppendDropdown(table, fields_poll_period, true, true, 'integer');
        
        this.fieldsAppendDropdown(table, fields_auto_firmware, true, false);
        this.fieldsAppendDropdown(table, fields_auto_config, true, false);
        
        this.fieldsAppendButton(table,   fields_buttons);
        
        if (skin != "airgain") {
            if (systemType == "ei") {
                this.fieldsAppendDropdown(table, fields_poe,   true, false, undefined, "Flash MCU...", this.handleMCUFlash);
            } 
          delayDropdown = this.fieldsAppendDropdown(table, fields_shutdown, (mcu_val == "vehicle"), false);
          //delayDropdown = this.fieldsAppendDropdown(table, fields_shutdown, true, false);
          //this.setShutdownEnabled(mcu_val == "vehicle");
        }  

        this.fieldsAppendButton(table,   fields_system);

        return table;
	}
});
