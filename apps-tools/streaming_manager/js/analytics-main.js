//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Danilyuk Nikolay
//-------------------------------------------------

(function($) {
    var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function() {
            AnalyticsCore.sendExecTime('/stream_manager', 'stream_manager');
            AnalyticsCore.sendScreenView('/stream_manager', 'Redpitaya', 'Stream control');
            AnalyticsCore.sendSysInfo('/stream_manager');
            startUsing = performance.now();
        });
    });

    $(window).on('beforeunload', function() {
        $.cookie('stream_manager-run', performance.now() - startUsing);
    });

})(jQuery);