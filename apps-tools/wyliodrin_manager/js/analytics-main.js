//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendExecTime('/wyliodrin_manager', 'wyliodrin_manager');
            AnalyticsCore.sendScreenView('/wyliodrin_manager', 'Redpitaya', 'Visual Programing');
            AnalyticsCore.sendSysInfo('/wyliodrin_manager');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('wyliodrin_manager-run', performance.now() - startUsing);
	});

})(jQuery);
