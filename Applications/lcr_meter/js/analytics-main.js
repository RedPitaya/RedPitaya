//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendScreenView('/lcr_meter', 'Redpitaya', 'LCR meter');
            AnalyticsCore.sendSysInfo('/lcr_meter');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      AnalyticsCore.sendTiming('/lcr_meter', 'App executed', 'LCR meter', performance.now() - startUsing);
	});

})(jQuery);
