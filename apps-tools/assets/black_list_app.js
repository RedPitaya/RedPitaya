//-------------------------------------------------
//      Redpitaya desktop
//      Created by Nikolay Danilyuk
//-------------------------------------------------

// The function removes incompatible applications
(function(Desktop, $) {

    Desktop.filterApps = function(listOfapplications,model) {
        if (model == "STEM 10") {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_z20_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        }

        if (model == "STEM 14") {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_z20_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        }

        if (model == "STEM 14-Z20") {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        }

        if (model == "STEM 16") {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'ba_pro' ||
                    listOfapplications[i]["id"] === 'lcr_meter' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'jupyter' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

        if (model == "STEM 250 12") {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'jupyter' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_z20_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

        if (model == "STEM 14-Z20-4CH") {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'ba_pro' ||
                    listOfapplications[i]["id"] === 'lcr_meter' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'jupyter' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_z20_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

        return listOfapplications;
    }

})(window.Desktop = window.Desktop || {}, jQuery);