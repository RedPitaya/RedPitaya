//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendExecTime('/wifi_wizard', 'wifi_wizard');
            AnalyticsCore.sendScreenView('/wifi_wizard', 'Redpitaya', 'Remote control');
            AnalyticsCore.sendSysInfo('/wifi_wizard');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('wifi_wizard-run', performance.now() - startUsing);
	});

})(jQuery);
