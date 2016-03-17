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
            AnalyticsCore.sendExecTime('/scpi_server', 'scpi_server');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('scpi_server-run', performance.now() - startUsing);
	});

})(jQuery);
