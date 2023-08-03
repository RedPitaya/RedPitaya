(function(OSC, $, undefined) {
    OSC.updateInterfaceFor250 = function(model) {
        if (model !== undefined) {
            if (OSC.rp_model == "") {
                if (model.value != "Z20_250_12" && model.value != "Z20_250_12_120") {
                    var nodes = document.getElementsByClassName("250_12_block");
                    [...nodes].forEach((element, index, array) => {
                        element.parentNode.removeChild(element);
                    });
                } else {
                    OSC.rp_model = model.value;
                    OSC.sample_rates = ['250M', '31.25M', '3.906M', '244.141k', '30.518k', '3.815k'];
                    OSC.max_freq  = 250e6;
                    var nodes = document.getElementsByClassName("speed_val");
                    [...nodes].forEach((element, index, array) => {
                        dec = parseInt(element.attributes.getNamedItem("value").value);
                        element.textContent = Math.round(2500 / dec)/10 + " MS/s";
                    });

                }
            }
        }
    };
}(window.OSC = window.OSC || {}, jQuery));