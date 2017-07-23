//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendExecTime('/scpi_manager', 'scpi_manager');
            AnalyticsCore.sendScreenView('/scpi_manager', 'Redpitaya', 'Remote control');
            AnalyticsCore.sendSysInfo('/scpi_manager');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('scpi_manager-run', performance.now() - startUsing);
	});

})(jQuery);
