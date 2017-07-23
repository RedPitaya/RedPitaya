//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendExecTime('/network_manager', 'network_manager');
            AnalyticsCore.sendScreenView('/network_manager', 'Redpitaya', 'Remote control');
            AnalyticsCore.sendSysInfo('/network_manager');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('network_manager-run', performance.now() - startUsing);
	});

})(jQuery);
