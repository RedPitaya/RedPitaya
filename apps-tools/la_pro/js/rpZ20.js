(function(OSC, $, undefined) {
    OSC.updateInterfaceForZ20 = function(model) {
        if (model !== undefined) {
            if (OSC.rp_model == "") {
                if (model.value === "Z20") {
                    OSC.rp_model = model.value;
                    OSC.sample_rates = ['122.8M', '15.35M', '1.919M', '119.921k', '14.99k', '1.874k'];
                    var nodes = document.getElementsByClassName("speed_val");
                    [...nodes].forEach((element, index, array) => {
                        dec = parseInt(element.attributes.getNamedItem("value").value);
                        element.textContent = Math.round(1228 / dec)/10 + " MS/s";
                    });
                    OSC.max_freq  = 122.880e6;
                }
            }
        }
    };
}(window.OSC = window.OSC || {}, jQuery));