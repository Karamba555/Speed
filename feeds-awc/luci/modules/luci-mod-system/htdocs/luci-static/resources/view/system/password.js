'use strict';
'require view';
'require dom';
'require ui';
'require form';
'require rpc';

NBP.init("mostcommon_1000", "/luci-static/resources/nbp/", true);

var formData = {
	password: {
		pw1: null,
		pw2: null
	}
};

var callSetPassword = rpc.declare({
	object: 'luci',
	method: 'setPassword',
	params: [ 'username', 'password' ],
	expect: { result: false }
});

return view.extend({
	checkPassword: function(section_id, value) {
		var strength = document.querySelector('.cbi-value-description'),
		    //strongRegex = new RegExp("^(?=.{10,})(?=.*[A-Z])(?=.*[a-z])(?=.*[0-9])(?=.*\\W).*$", "g"),
                    strongRegex = new RegExp("^(?=.{10,})(?=.*[A-Z])(?=.*[a-z])(?=.*[0-9])", "g"),
		    mediumRegex = new RegExp("^(?=.{10,})(((?=.*[A-Z])(?=.*[a-z]))|((?=.*[A-Z])(?=.*[0-9]))|((?=.*[a-z])(?=.*[0-9]))).*$", "g"),
		    enoughRegex = new RegExp("(?=.{10,}).*", "g");

		if (strength && value.length) {
			if (false == enoughRegex.test(value)) {
				strength.innerHTML = '%s: <span style="color:red">%s</span>'.format(_('Password strength'), _('More Characters'));
				return "More Characters";
			} else if (strongRegex.test(value)) {
				strength.innerHTML = '%s: <span style="color:green">%s</span>'.format(_('Password strength'), _('Strong'));
                	} else if (mediumRegex.test(value)) {
				strength.innerHTML = '%s: <span style="color:orange">%s</span>'.format(_('Password strength'), _('Medium'));
                   		return "Medium Complexity";
            		} else {
				strength.innerHTML = '%s: <span style="color:red">%s</span>'.format(_('Password strength'), _('Weak'));
                		return "Weak Complexity";
            		}
		}

		return true;
	},

	load: function() {
		return Promise.all([
		    L.resolveDefault(L.uci.load('awc_cloud'))
		])
	},

	render: function() {
		var m, s, o;

        var eula = L.uci.get('awc_cloud', 'cloud', 'eula');

        if (eula != "yes_pass") {
          ui.addNotification(null, E('p', _('The system password must be changed after first login.')), 'info');
        }

		m = new form.JSONMap(formData, _('Router Password'), _('Changes the administrator password for accessing the device'));
		m.readonly = !L.hasViewPermission();

		s = m.section(form.NamedSection, 'password', 'password');

		o = s.option(form.Value, 'pw1', _('Password'));
		o.password = true;
		o.validate = this.checkPassword;

		o = s.option(form.Value, 'pw2', _('Confirmation'), ' ');
		o.password = true;
		o.renderWidget = function(/* ... */) {
			var node = form.Value.prototype.renderWidget.apply(this, arguments);

			node.querySelector('input').addEventListener('keydown', function(ev) {
				if (ev.keyCode == 13 && !ev.currentTarget.classList.contains('cbi-input-invalid'))
					document.querySelector('.cbi-button-save').click();
			});

			return node;
		};

		return m.render();
	},

	handleSave: function() {
		var map = document.querySelector('.cbi-map');

		return dom.callClassMethod(map, 'save').then(function() {
			if (formData.password.pw1 == null || formData.password.pw1.length == 0)
				return;

			if (formData.password.pw1 != formData.password.pw2) {
				ui.addNotification(null, E('p', _('Given password confirmation did not match, password not changed!')), 'danger');
				return;
			}

			if (NBP.isCommonPassword(formData.password.pw1)) {
				ui.addNotification(null, E('p', _('Given password is too common')), 'danger');
				return;
			}

			return callSetPassword('root', formData.password.pw1).then(function(success) {
				if (success) {
					//ui.addNotification(null, E('p', _('The system password has been successfully changed.')), 'info');

				    L.uci.set('awc_cloud', 'cloud', 'eula', 'yes_pass');
				    L.uci.save();
				    L.uci.apply(0);

				    setTimeout(()=> {
				      window.location.href = "/cgi-bin/luci/";
				    },1500);

				} else {
					ui.addNotification(null, E('p', _('Failed to change the system password.')), 'danger');
				}

				formData.password.pw1 = null;
				formData.password.pw2 = null;

				dom.callClassMethod(map, 'render');
			});
		});
	},

	handleSaveApply: null,
	handleReset: null
});
