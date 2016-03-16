//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendScreenView('/scpi_server', 'Redpitaya', 'Remote control');
            AnalyticsCore.sendSysInfo('/scpi_server');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      AnalyticsCore.sendTiming('/scpi_server', 'App executed', 'Remote control', performance.now() - startUsing);
	});

})(jQuery);
