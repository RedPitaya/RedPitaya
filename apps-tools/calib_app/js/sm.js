(function() {
    var originalAddClassMethod = jQuery.fn.addClass;
    var originalRemoveClassMethod = jQuery.fn.removeClass;
    $.fn.addClass = function(clss) {
        var result = originalAddClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'add');
        return result;
    };
    $.fn.removeClass = function(clss) {
        var result = originalRemoveClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'remove');
        return result;
    }
})();


(function(SM, $, undefined) {


    SM.param_callbacks = {};

    SM.rp_model = "";


    // App state
    SM.state = {
        processing: false,
        cursor_dragging: false,
        mouseover: false
    };


    //Write email
    SM.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: CLIENT.parametersCache }) + "%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

        var url = 'info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            body += " info.json: " + "%0D%0A" + msg.responseText;
        }).fail(function(msg) {
            var info_json = msg.responseText
            var ver = '';
            try {
                var obj = JSON.parse(msg.responseText);
                ver = " " + obj['version'];
            } catch (e) {};

            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
        });
    }


    // For Firefox
    function fireEvent(obj, evt) {
        var fireOnThis = obj;
        if (document.createEvent) {
            var evObj = document.createEvent('MouseEvents');
            evObj.initEvent(evt, true, false);
            fireOnThis.dispatchEvent(evObj);

        } else if (document.createEventObject) {
            var evObj = document.createEventObject();
            fireOnThis.fireEvent('on' + evt, evObj);
        }
    }




}(window.SM = window.SM || {}, jQuery));

function clamp(value, min, max) {
  return Math.min(Math.max(value, min), max);
}


function scaleContainers() {
    const container = document.querySelector('#adc_mode_body > div');
    const holder = document.querySelector('#adc_mode_body > div > div');
    const availableH = container.clientHeight - 50;
    const availableW = container.clientWidth - 200;
    
    var scale = clamp(availableH / 610,1,1.5);
    scale = clamp( availableW / 1500,1,1.5);
    holder.style.transform = `scale(${scale})`;
    holder.style.transformOrigin = 'top center';
    container.style.height = (holder.clientHeight + 50) * scale +'px'
    if (scale > 1){
        container.style.overflowX = 'hidden';
    }else{
        container.style.overflowX = 'auto';
    }
}

// Page onload event handler
$(function() {

    var reloaded = $.cookie("SM_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("SM_forced_reload", "true");
        window.location.reload(true);
    }


    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        SM.ws.onclose = function() {}; // disable onclose handler first
        SM.ws.close();
        $.ajax({
            url: SM.config.stop_app_url,
            async: false
        });
    });

    $(window).resize(function() {
        OBJ.cursorResize();
        scaleContainers();
        SW.checkSubWindowPosition()
    });


    //Crash buttons
    $('#send_report_btn').on('click', function() { SM.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });


});