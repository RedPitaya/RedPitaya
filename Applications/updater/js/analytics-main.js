//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendScreenView('/updater', 'Redpitaya', 'Remote control');
            AnalyticsCore.sendSysInfo('/updater');
            AnalyticsCore.sendExecTime('/updater', 'updater');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('updater-run', performance.now() - startUsing);
	});

})(jQuery);
