

/**
 * Process signal value and return percentage for an icon and 
 * descriptive test. TODO: Add translation from dB signal value. 
 */


function getSignalFromPercent(signalPercent, homeNetwork) {
    var signal = new Object();
    
    if (homeNetwork == undefined || homeNetwork == "") {
        signal.text = _("Not connected");
        signal.percent = 0;
                   
    } else if (signalPercent <= 40) {
        signal.text = _("Poor");
        signal.percent = 0;
                    
    } else if (signalPercent > 40 && signalPercent <= 55) {
        signal.text = _("Fair");
        signal.percent = 25;
                    
    } else if (signalPercent > 55 && signalPercent <= 70) {
        signal.text = _("OK");
        signal.percent = 50;
                    
    } else if (signalPercent > 70 && signalPercent <= 85) {
        signal.text = _("Good");
        signal.percent = 75; 
                    
    } else {
        signal.text = _("Excellent");
        signal.percent = 100;   
    }
    
    return signal;
}


/**
 * From:
 * 
 * https://www.digi.com/support/knowledge-base/understanding-lte-signal-strength-values
 */

function getSignalFromRSRP(rsrp, homeNetwork) {
    var signal = new Object();
    
    if (rsrp == undefined || homeNetwork == undefined || homeNetwork == "") {
        signal.text = _("Not connected");
        signal.percent = 0;
                   
    } else if (rsrp <= -100) {
        signal.text = _("Poor");
        signal.percent = 25;
                    
    } else if (rsrp > -100 && rsrp <= -90) {
        signal.text = _("Fair");
        signal.percent = 50;
                    
    } else if (rsrp > -90 && rsrp < -80) {
        signal.text = _("Good");
        signal.percent = 75; 
                    
    } else {
        signal.text = _("Excellent");
        signal.percent = 100;   
    }
    
    return signal; 
    
}


function getIcon(filename, size) {
    if (size == undefined) size = 100;
    
    return "<img src=\"" + L.resource('icons/' + filename) + "\" width=\"" + size + "\">";
}
