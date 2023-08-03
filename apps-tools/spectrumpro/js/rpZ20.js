(function(SPEC, $, undefined) {
    SPEC.updateInterfaceForZ20 = function(model) {
        if (model !== undefined) {
                if (model === "Z20") {
                    SPEC.rp_model = model;
                    SPEC.config.xmax = 122/2;
                    $("#SOUR1_FREQ_FIX").attr("max", 122.880e6/2);
                    $("#SOUR2_FREQ_FIX").attr("max", 122.880e6/2);
                    $("#SOUR1_VOLT").attr("max", 0.5);
                    $("#SOUR2_VOLT").attr("max", 0.5);
                    $("#SOUR1_VOLT_OFFS").attr("max", 0.5);
                    $("#SOUR2_VOLT_OFFS").attr("max", 0.5);
                    $("#SOUR1_VOLT_OFFS").attr("min", -0.5);
                    $("#SOUR2_VOLT_OFFS").attr("min", -0.5);

                    var nodes = document.getElementsByClassName("122_16_block_remove");
                    [...nodes].forEach((element, index, array) => {
                            element.parentNode.removeChild(element);
                        });

                    $(".122_16_gen_sel").prop( "disabled", true );
                    $(".122_16_gen_sel").css({ "-webkit-appearance": "none","-moz-appearance": "none", "appearance": "none" });
                    $(".out_phase").switchClass("col-xs-6", "col-xs-12");
                }
        }
    };
}(window.SPEC = window.SPEC || {}, jQuery));