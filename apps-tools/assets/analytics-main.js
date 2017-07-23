//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendScreenView('/', 'Redpitaya', 'Main menu');
            AnalyticsCore.sendSysInfo('/');
        });
    });
})(jQuery);
