//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
    var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function() {
            AnalyticsCore.sendScreenView('/scopegenpro', 'Redpitaya', 'Oscilloscope Pro');
            AnalyticsCore.sendSysInfo('/scopegenpro');
            AnalyticsCore.sendExecTime('/scopegenpro', 'scopegenpro');
            startUsing = performance.now();
        });
    });

    $(window).on('beforeunload', function(e) {
        $.cookie('scopegenpro-run', performance.now() - startUsing);
    });

})(jQuery);
