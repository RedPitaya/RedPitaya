//-------------------------------------------------
//      Redpitaya network status checker
//      Created by Artem Kokos
//-------------------------------------------------

(function(OnlineChecker, $) {
    OnlineChecker.online = false;
    OnlineChecker.isOnline = function() {
        return OnlineChecker.online;
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
        });
        Offline.on('down', function() {
            OnlineChecker.online = false;
        });
        Offline.check();
        var run = function() {
            Offline.check();
        }
        setInterval(run, 8000);
        $(window).unload(function() {
            var date = new Date();
            date.setTime(date.getTime() + (10 * 1000)); // 10 seconds
            $.cookie('online_status', OnlineChecker.online, { expires: date, path: '/' });
            return null;
        });
    });


})(window.OnlineChecker = window.OnlineChecker || {}, jQuery);
