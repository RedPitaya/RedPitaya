//-------------------------------------------------
//      Redpitaya network status checker
//      Created by Artem Kokos
//-------------------------------------------------

(function(OnlineChecker, $) {
    OnlineChecker.online = false;
    OnlineChecker.callback = undefined;

    OnlineChecker.isOnline = function() {
        return OnlineChecker.online;
    }

    OnlineChecker.checkAsync = function(callback) {
        OnlineChecker.callback = callback;
        Offline.check();
    }

    $(document).ready(function($) {
        if ($.cookie !== undefined) {
            if ($.cookie('online_status') !== undefined) {
                if ($.cookie('online_status') == "true")
                    Offline.markUp();
                else
                    Offline.markDown();
            } else Offline.markDown();
        }
        Offline.options = {
            checks: {
                xhr: {
                    url: '/check_inet'
                }
            },
            checkOnLoad: false,
        };
        Offline.on('up', function() {
            OnlineChecker.online = true;
            if (OnlineChecker.callback != undefined)
                OnlineChecker.callback();
        });
        Offline.on('down', function() {
            OnlineChecker.online = false;
            if (OnlineChecker.callback != undefined)
                OnlineChecker.callback();
        });
        //Offline.check();
        var run = function() {
                Offline.check();
            }
            // setInterval(run, 8000);
        $(window).unload(function() {
            var date = new Date();
            date.setTime(date.getTime() + (15 * 1000)); // 10 seconds
            $.cookie('online_status', OnlineChecker.online, { expires: date, path: '/' });
            return null;
        });
    });


})(window.OnlineChecker = window.OnlineChecker || {}, jQuery);
