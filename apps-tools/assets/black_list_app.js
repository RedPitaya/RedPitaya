//-------------------------------------------------
//      Redpitaya desktop
//      Created by Nikolay Danilyuk
//-------------------------------------------------

// The function removes incompatible applications
(function(Desktop, $) {
  
    Desktop.filterApps = function(listOfapplications,model) {
//       STEM_125_10_v1_0            = 0,
        if (model == 0) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        }
//  STEM_125_14_v1_0            = 1,
//  STEM_125_14_v1_1            = 2,
//  STEM_125_14_LN_v1_1         = 5,
//  STEM_125_14_LN_BO_v1_1      = 17,
//  STEM_125_14_LN_CE1_v1_1     = 18,
//  STEM_125_14_LN_CE2_v1_1     = 19,

            if (model == 1 || model == 2 || model == 5 || model == 17 || model == 18 || model == 19) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        }

//  STEM_125_14_Z7020_v1_0      = 6,
//  STEM_125_14_Z7020_LN_v1_1   = 7,

        if (model == 6 || model == 7) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        }

//  STEM_122_16SDR_v1_0         = 3,
//  STEM_122_16SDR_v1_1         = 4,

        if (model == 3 || model == 4) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'ba_pro' ||
                    listOfapplications[i]["id"] === 'lcr_meter' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'impedance_analyzer' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

//  STEM_250_12_v1_0            = 11,
//  STEM_250_12_v1_1            = 12,
//  STEM_250_12_v1_2            = 13,
//  STEM_250_12_120             = 14,
//  STEM_250_12_v1_2a           = 15,
//  STEM_250_12_v1_2b           = 16,

        if (model == 11 || model == 12 || model == 13 || model == 14 || model == 15 || model == 16) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

//  STEM_125_14_Z7020_4IN_v1_0  = 8,
//  STEM_125_14_Z7020_4IN_v1_2  = 9,
//  STEM_125_14_Z7020_4IN_v1_3  = 10,

        if (model == 8 || model == 9 || model == 10) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'ba_pro' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'arb_manager' ||
                    listOfapplications[i]["id"] === 'lcr_meter' ||
                    listOfapplications[i]["id"] === 'impedance_analyzer' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

//  STEM_125_14_v2_0            = 20,
//  STEM_125_14_Pro_v2_0        = 21,

        if (model == 20 || model == 21) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'streaming_manager' ||
                    listOfapplications[i]["id"] === 'la_pro' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

//  STEM_125_14_Z7020_Pro_v2_0  = 22,
//  STEM_125_14_Z7020_Ind_v2_0  = 23,
//  STEM_125_14_Z7020_Pro_v1_0  = 24,

        if (model == 22 || model == 23 || model == 24) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'streaming_manager' ||
                    listOfapplications[i]["id"] === 'la_pro' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };


//  STEM_125_14_Z7020_LL_v1_1   = 25,
//  STEM_125_14_Z7020_LL_v1_2   = 27,
//  STEM_125_14_Z7020_TI_v1_3   = 28,

        if (model == 25 || model == 27 || model == 28) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'streaming_manager' ||
                    listOfapplications[i]["id"] === 'la_pro' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };


//  STEM_65_16_Z7020_LL_v1_1    = 26,
//  STEM_65_16_Z7020_TI_v1_3    = 29,

        if (model == 26 || model == 29) {
            for (i = listOfapplications.length - 1; i >= 0; i -= 1) {
                if (listOfapplications[i]["id"] === 'marketplace' ||
                    listOfapplications[i]["id"] === 'streaming_manager' ||
                    listOfapplications[i]["id"] === 'la_pro' ||
                    listOfapplications[i]["id"] === 'pyrpl' ||
                    listOfapplications[i]["id"] === 'fpgaexamples' ||
                    listOfapplications[i]["id"] === 'activelearning' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_receiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_hpsdr_122_88' ||
                    listOfapplications[i]["id"] === 'vna' ||
                    listOfapplications[i]["id"] === 'vna_z20' ||
                    listOfapplications[i]["id"] === 'vna_122_88' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_z20' ||
                    listOfapplications[i]["id"] === 'sdr_transceiver_122_88') {
                    listOfapplications.splice(i, 1);
                }
            }
        };

        return listOfapplications;
    }

})(window.Desktop = window.Desktop || {}, jQuery);
