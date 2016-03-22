//-------------------------------------------------
//      Redpitaya network status checker
//      Created by Artem Kokos
//-------------------------------------------------

(function(OnlineChecker, $) {
    OnlineChecker.online = true;

    OnlineChecker.isOnline = function() {
        return OnlineChecker.online;
    }

    $(document).ready(function($) {
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
        setInterval(run, 3000);
    });

})(window.OnlineChecker = window.OnlineChecker || {}, jQuery);