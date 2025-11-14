/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(SPEC, $, undefined) {
    SPEC.updateInterfaceForZ20 = function(model) {
        if (model !== undefined) {
                if (model === "Z20_122_16") {
                    SPEC.rp_model = model;

                    var nodes = document.getElementsByClassName("122_16_block_remove");
                    [...nodes].forEach((element, index, array) => {
                            element.parentNode.removeChild(element);
                        });

                    $(".out_phase").switchClass("col-xs-6", "col-xs-12");
                }
        }
    };
}(window.SPEC = window.SPEC || {}, jQuery));