//-------------------------------------------------
//      Redpitaya analytics core system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function(AnalyticsCore, $) {
    var analytics_id = "UA-75663037-1";
    var location = 'www.redpitaya.com';
    var sendData = false;
    var debugPrints = false;

    (function() {
        if ("performance" in window == false) {
            window.performance = {};
        }

        Date.now = (Date.now || function() { // thanks IE8
            return new Date().getTime();
        });

        if ("now" in window.performance == false) {
            var nowOffset = Date.now();
            if (performance.timing && performance.timing.navigationStart) {
                nowOffset = performance.timing.navigationStart
            }
            window.performance.now = function now() {
                return Date.now() - nowOffset;
            }
        }

    })();
    /**
     * Creates a temporary global ga object and loads analy  tics.js.
     * Paramenters o, a, and m are all used internally.  They could have been declared using 'var',
     * instead they are declared as parameters to save 4 bytes ('var ').
     *
     * @param {Window}      i The global context object.
     * @param {Document}    s The DOM document object.
     * @param {string}      o Must be 'script'.
     * @param {string}      g URL of the analytics.js script. Inherits protocol from page.
     * @param {string}      r Global name of analytics object.  Defaults to 'ga'.
     * @param {DOMElement?} a Async script tag.
     * @param {DOMElement?} m First script tag in document.
     */
    (function(i, s, o, g, r, a, m) {
        i['GoogleAnalyticsObject'] = r; // Acts as a pointer to support renaming.

        // Creates an initial ga() function.  The queued commands will be executed once analytics.js loads.
        i[r] = i[r] || function() {
                (i[r].q = i[r].q || []).push(arguments)
            },

            // Sets the time (as an integer) this tag was executed.  Used for timing hits.
            i[r].l = 1 * new Date();

        // Insert the script tag asynchronously.  Inserts above current tag to prevent blocking in
        // addition to using the async attribute.
        a = s.createElement(o),
            m = s.getElementsByTagName(o)[0];
        a.async = 1;
        a.src = g;
        m.parentNode.insertBefore(a, m)
    })(window, document, 'script', '//www.google-analytics.com/analytics.js', 'ga');


    AnalyticsCore.sendSysInfo = function(page) {
        if (!sendData) return;
        var url_arr = window.location.href.split("/");
        var url = url_arr[0] + '//' + url_arr[2] + '/info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            if (debugPrints) console.log("[AnalyticsCore] sendSysInfo", page);
            var info = JSON.parse(msg);
            $('#ver').html(info['description'].substring(0, info['description'].length - 1) + ' ' + info['version']);
            AnalyticsCore.sendEvent(page, 'info', 'Revision', info['revision']);
            AnalyticsCore.sendEvent(page, 'info', 'Version', info['version']);
        }).fail(function(msg) {
            if (debugPrints) console.log("[AnalyticsCore] sendSysInfo", page);
            var info = JSON.parse(msg.responseText);
            $('#ver').html(info['description'].substring(0, info['description'].length - 1) + ' ' + info['version']);
            if(info['version'].startsWith('0.94'))
            {
                AnalyticsCore.sendData = false;
                return;
            }
            AnalyticsCore.sendEvent(page, 'info', 'Revision', info['revision']);
            AnalyticsCore.sendEvent(page, 'info', 'Version', info['version']);
        });
    }

    AnalyticsCore.sendEvent = function(page, event_category, event_action, event_label) {
        if (!sendData) return;
        if (debugPrints) console.log("[AnalyticsCore] sendEvent", page, event_category, event_action, event_label);
        ga('send', {
            location: location,
            page: page,
            hitType: 'event',
            eventCategory: event_category,
            eventAction: event_action,
            eventLabel: event_label
        });
    }

    AnalyticsCore.sendEventWithValue = function(page, event_category, event_action, event_label, event_value) {
        if (!sendData) return;
        if (debugPrints) console.log("[AnalyticsCore] sendEvent", page, event_category, event_action, event_label, parseInt(event_value));
        ga('send', {
            location: location,
            page: page,
            hitType: 'event',
            eventCategory: event_category,
            eventAction: event_action,
            eventLabel: event_label,
            eventValue: parseInt(event_value)
        });
    }

    AnalyticsCore.sendTiming = function(page, timing_category, timing_var, timing_val) {
        if (!sendData) return;
        if (debugPrints) console.log("[AnalyticsCore] sendTiming", page, timing_category, timing_var, timing_val);
        ga('send', {
            location: location,
            page: page,
            hitType: 'timing',
            timingCategory: timing_category,
            timingVar: timing_var,
            timingValue: parseInt(timing_val)
        });
    }

    AnalyticsCore.sendExecTime = function(page, app) {
        if ($.cookie(app + '-run') !== undefined) {
            AnalyticsCore.sendEventWithValue(page, 'info', 'Execution time', $.cookie(app + '-run'), $.cookie(app + '-run'));
            AnalyticsCore.sendTiming(page, "App execution time", app, $.cookie(app + '-run'));
        }

    }

    AnalyticsCore.sendScreenView = function(page, app_name, screen_name) {
        if (!sendData) return;
        if (debugPrints) console.log("[AnalyticsCore] sendScreenView", page, app_name, screen_name);
        ga('send', 'screenview', {
            location: location,
            page: page,
            appName: app_name,
            screenName: screen_name
        });
    }

    AnalyticsCore.init = function(successCallback) {
        ga('create', analytics_id, 'auto');
        var stat = $.cookie('send_stats');
        if (stat === undefined) {
            $('#analytics_dialog').modal('show');
            $('#enable_analytics').click(function(event) {
                window['ga-disable-' + analytics_id] = false;
                var date = new Date();
                var minutes = 30;
                date.setTime(date.getTime() + (14 * 24 * 60 * 60 * 1000)); // 2 weeks

                $.cookie('send_stats', true, { expires: date });
                sendData = true;
                if (debugPrints) console.log("[AnalyticsCore] Analytics enabled");
                successCallback();
            });
            $('#disable_analytics').click(function(event) {
                var date = new Date();
                var minutes = 30;
                date.setTime(date.getTime() + (14 * 24 * 60 * 60 * 1000)); // 2 weeks

                window['ga-disable-' + analytics_id] = true;
                $.cookie('send_stats', false, { expires: date });
                if (debugPrints) console.log("[AnalyticsCore] Analytics disabled");
                sendData = false;
            });

        } else if (stat == "false") {
            if (debugPrints) console.log("[AnalyticsCore] Analytics disabled");
            sendData = false;
            window['ga-disable-' + analytics_id] = true;
        } else {
            if (debugPrints) console.log("[AnalyticsCore] Analytics enabled");
            sendData = true;
            successCallback();
        }
    };

    AnalyticsCore.sleep = function(milliseconds) {
        var start = new Date().getTime();
        for (var i = 0; i < 1e7; i++) {
            if ((new Date().getTime() - start) > milliseconds) {
                break;
            }
        }
    }

})(window.AnalyticsCore = window.AnalyticsCore || {}, jQuery);
