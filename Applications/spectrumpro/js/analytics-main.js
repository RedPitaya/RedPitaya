//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendScreenView('/spectrumpro', 'Redpitaya', 'Spectrum analyser Pro');
            AnalyticsCore.sendSysInfo('/spectrumpro');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      AnalyticsCore.sendTiming('/spectrumpro', 'App executed', 'Spectrum analyser Pro', performance.now() - startUsing);
	});

})(jQuery);
