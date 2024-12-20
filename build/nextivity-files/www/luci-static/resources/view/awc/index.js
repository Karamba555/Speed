'use strict';
'require view';
'require dom';
'require poll';
'require fs';
'require network';

function invokeIncludesLoad(includes) {
	var tasks = [], has_load = false;

	for (var i = 0; i < includes.length; i++) {
		if (typeof(includes[i].load) == 'function') {
			tasks.push(includes[i].load().catch(L.bind(function() {
				this.failed = true;
			}, includes[i])));

			has_load = true;
		}
		else {
			tasks.push(null);
		}
	}

	return has_load ? Promise.all(tasks) : Promise.resolve(null);
}

function startPolling(includes, containers, dynamic) {
	var step = function() {
		return network.flushCache().then(function() {
			return invokeIncludesLoad(includes);
		}).then(function(results) {
			for (var i = 0; i < includes.length; i++) {
				var content = null;

				if (includes[i].failed)
					continue;

				if (typeof(includes[i].render) == 'function')
					content = includes[i].render(results ? results[i] : null);
				else if (includes[i].content != null)
					content = includes[i].content;

				if (content != null) {
                    if (containers[i]) {
                        containers[i].parentNode.style.display = '';
                        containers[i].parentNode.classList.add('fade-in');

                        dom.content(containers[i], content);
                    }
				}
			}

/*			var ssi = dynamic ? document.querySelector('div.includes-dynamic') :
                                document.querySelector('div.includes-static');*/
            var ssi = document.querySelector('div.includes');

			if (ssi) {
				ssi.style.display = '';
				ssi.classList.add('fade-in');
			}
		});
	};
    
	return step().then(function() {
		if (dynamic) poll.add(step);
	});
}

return view.extend({
	load: function() {
		return L.resolveDefault(fs.list('/www' + L.resource('view/awc/include')), []).then(function(entries) {
			return Promise.all(entries.filter(function(e) {
				return (e.type == 'file' && e.name.match(/\.js$/));
			}).map(function(e) {
				return 'view.awc.include.' + e.name.replace(/\.js$/, '');
			}).sort().map(function(n) {
				return L.require(n);
			}));
		});
	},

	render: function(includes) {
		var rv = E([]);
        var containers_dynamic = [], containers_static = [];
        var includes_dynamic   = [], includes_static   = [];
        
		for (var i = 0; i < includes.length; i++) {
			var title = null;
            var id = null;
            
            //console.log(String(includes[i]));
            
			if (includes[i].title != null)
				title = includes[i].title;
			else
				title = String(includes[i]).replace(/^\[ViewAwcInclude\d+_(.+)Class\]$/,
					function(m, n) { return n.replace(/(^|_)(.)/g,
						function(m, s, c) { return (s ? ' ' : '') + c.toUpperCase() })
					});

            if (includes[i].id != null) {
                id = includes[i].id;
            } else {
                id = "include " + i;
            }
                
			//var container = E('div', { 'class': 'section-body', 'style': 'margin-left:2em'});
            var container = E('div', { 'class': 'section-body section-indent'});
            
            // Force dummy sections
            var heading; // = 'h2'
            var isTitle = (title == "Networking" || title == "Admin Tools");
            
            if (isTitle) {
                heading = 'h2';
            } else {
                heading = 'h3';
                title = "&bull; " + title;
            }

			rv.appendChild(E('div', { 'class': 'cbi-section', 'style': 'display:none', 'id': id }, [
				title != '' ? E(heading, title) : '',
                isTitle ? 
                    E('hr', { 'class': 'title-section'}) :
                    E('hr', { 'class': 'status-section'}),
				container
			]));

            if (!id.includes("admin")) {
                containers_dynamic.push(container);
                includes_dynamic.push(includes[i]);
            } else {
                containers_static.push(container);
                includes_static.push(includes[i]);
            }
		}

		return startPolling(includes_dynamic, containers_dynamic, true).then(function() {
            startPolling(includes_static, containers_static, false);
			return rv;
		});
	},

	//handleSaveApply: null,
	//handleSave: null,
	//handleReset: null
});
