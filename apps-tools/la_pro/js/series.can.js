(function(CAN, $, undefined) {
    CAN.ANNOTATIONS = {
		PAYLOAD_DATA 	: 0,  // 'Payload data'
		START_OF_FRAME 	: 1,  // 'Start of frame'
		END_OF_FRAME 	: 2,  // 'End of frame'
		ID 				: 3,  // 'Identifier'
		EXT_ID 			: 4,  // 'Extended identifier'
		FULL_ID 		: 5,  // 'Full identifier'
		IDE 			: 6,  // 'Identifier extension bit'
		RESERV_BIT		: 7,  // 'Reserved bit 0 and 1'
		RTR 			: 8,  // 'Remote transmission request'
		SRR				: 9,  // 'Substitute remote request'
		DLC             : 10, // 'Data length count'
		CRC_DELIMITER   : 11, // 'CRC delimiter'
		ACK_SLOT		: 12, // 'ACK slot'
		ACK_DELIMITER	: 13, // 'ACK delimiter'
		STUFF_BIT	 	: 14, // 'Stuff bit'
		ERROR_3			: 15, // 'End of frame (EOF) must be 7 recessive bits'
		WARNING_1       : 16, // 'Identifier bits 10..4 must not be all recessive'
		WARNING_2		: 17, // 'CRC delimiter must be a recessive bit'
		WARNING_3		: 18, // 'ACK delimiter must be a recessive bit'
		BRS				: 19, // 'Bit rate switch'
		ESI 			: 20, // 'Error state indicator'
		RESERV_BIT_FLEX	: 21, // 'Flexible data'
		STUFF_BIT_ERROR	: 22, // 'Stuff bit error'
		CRC_15_VAL		: 23, // CRC-15 Value
		CRC_17_VAL		: 24, // CRC-17 Value
		CRC_21_VAL		: 25, // CRC-21 Value
		FSB				: 26, // fixed stuff-bit (FSB)
		SBC				: 27, // Stuff bits before CRC in FD mode
		CRC_FSB_SBC		: 29, // Stuff bits before CRC in FD mode + FSB + CRC

    };

    CAN.data_arr = [];

    CAN.colors = {
        PAYLOAD_DATA: "#c9c25d",
        START_OF_FRAME: "#243cf0",
        END_OF_FRAME: "#243cf0",
        ID: "#32c21f",
        STUFF_BIT:"#94670d",
        FSB:"#94670d",
        RTR:"#cfccc8",
        BRS:"#cfccc8",
        ESI:"#cfccc8",
        SRR:"#cfccc8",
        RESERV_BIT:"#cfccc8",
        RESERV_BIT_FLEX:"#cfccc8",
        IDE:"#b5c4b5",
        DLC:"#a82895",
        CRC_15_VAL: "#d1323f",
        CRC_17_VAL: "#d1323f",
        CRC_21_VAL: "#d1323f",
        SBC: "#a87b28",
        CRC_DELIMITER:"#cfccc8",
        ACK_SLOT:"#cfccc8",
        ACK_DELIMITER:"#cfccc8",
        CRC_FSB_SBC: "#d1323f",
    }

    CAN.getAnnotation = function(value){
        return SERIES.decoderAnno.CAN[value]
    }

    CAN.getValue = function(item,radix){
        if (item.c == CAN.ANNOTATIONS.START_OF_FRAME) {
            return "Start";
        }

        if (item.c == CAN.ANNOTATIONS.END_OF_FRAME) {
            return COMMON.formatData(item.d, "End: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.PAYLOAD_DATA) {
            return COMMON.formatData(item.d, "Data:", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.ID) {
            return COMMON.formatData(item.d, "ID: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.EXT_ID) {
            return COMMON.formatData(item.d, "Ext ID: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.FULL_ID) {
            return COMMON.formatData(item.d, "FULL ID: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.STUFF_BIT) {
            return "Stuff bit"
        }

        if (item.c == CAN.ANNOTATIONS.FSB) {
            return "Fixed stuff bit"
        }

        if (item.c == CAN.ANNOTATIONS.RTR) {
            return "RTR"
        }

        if (item.c == CAN.ANNOTATIONS.SRR) {
            return "SRR"
        }

        if (item.c == CAN.ANNOTATIONS.RESERV_BIT) {
            return "Reserve bit"
        }

        if (item.c == CAN.ANNOTATIONS.IDE) {
            return "IDE"
        }

        if (item.c == CAN.ANNOTATIONS.BRS) {
            return "Bit rate switch"
        }

        if (item.c == CAN.ANNOTATIONS.ESI) {
            return "Error state indicator"
        }

        if (item.c == CAN.ANNOTATIONS.RESERV_BIT_FLEX) {
            return "Enable FD mode"
        }

        if (item.c == CAN.ANNOTATIONS.STUFF_BIT_ERROR) {
            return "Stuff bit error"
        }

        if (item.c == CAN.ANNOTATIONS.DLC) {
            return COMMON.formatData(item.d,"DLC: ", "", radix )[0]
        }

        if (item.c == CAN.ANNOTATIONS.CRC_15_VAL) {
            return COMMON.formatData(item.d,"CRC-15: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.CRC_17_VAL) {
            return COMMON.formatData(item.d,"CRC-17: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.CRC_21_VAL) {
            return COMMON.formatData(item.d,"CRC-21: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.SBC) {
            return COMMON.formatData(item.d,"SBC: ", "", radix)[0]
        }

        if (item.c == CAN.ANNOTATIONS.CRC_DELIMITER) {
            return "CRC delimiter"
        }

        if (item.c == CAN.ANNOTATIONS.ACK_SLOT) {
            return "ACK"
        }

        if (item.c == CAN.ANNOTATIONS.ACK_DELIMITER) {
            return "ACK delimiter"
        }

        if (item.c == CAN.ANNOTATIONS.CRC_FSB_SBC) {
            return COMMON.formatData(item.d,"Full CRC: ", "", radix)[0]
        }

    }

    CAN.drawDecoded = function(plot, canvascontext, offset, decoded_data) {
        var axes = plot.getAxes();
        var min = axes.xaxis.options.min
        var max = axes.xaxis.options.max
        var dd = decoded_data;
        for (var i = 0; i < dd.length; i++) {
            if (dd[i].s + dd[i].l < min) continue;
            if (dd[i].s > max) continue;

            if (dd[i].c == CAN.ANNOTATIONS.START_OF_FRAME) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.START_OF_FRAME, "S");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.START_OF_FRAME,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.END_OF_FRAME) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.END_OF_FRAME, COMMON.formatData(dd[i].d, "End: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.END_OF_FRAME,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.PAYLOAD_DATA) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.PAYLOAD_DATA, COMMON.formatData(dd[i].d, "", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.PAYLOAD_DATA,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ID) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.ID, COMMON.formatData(dd[i].d, "ID: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.ID,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.EXT_ID) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.ID, COMMON.formatData(dd[i].d, "Ext ID: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.ID,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.STUFF_BIT) {
                SERIES.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.STUFF_BIT, ["Stuff"])
                SERIES.drawBitsBarsTop(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.STUFF_BIT,dd[i].b, false)
            }

            if (dd[i].c == CAN.ANNOTATIONS.STUFF_BIT_ERROR) {
                SERIES.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.STUFF_BIT, ["Stuff error"])
                SERIES.drawBitsBarsTop(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.STUFF_BIT,dd[i].b, false)
            }

            if (dd[i].c == CAN.ANNOTATIONS.FSB) {
                SERIES.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.FSB, ["FSB"])
                SERIES.drawBitsBarsTop(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.FSB, dd[i].b, false)
            }

            if (dd[i].c == CAN.ANNOTATIONS.RTR) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RTR, ["RTR"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RTR,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.BRS) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.BRS, ["BRS"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.BRS,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ESI) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.ESI, ["ESI"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.ESI,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.SRR) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.SRR, ["SRR"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.SRR,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.RESERV_BIT) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["Rs"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.IDE) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.IDE, ["IDE"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.IDE,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.RESERV_BIT_FLEX) {
                SERIES.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.STUFF_BIT, ["FD mode"])
            }

            if (dd[i].c == CAN.ANNOTATIONS.DLC) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.DLC, COMMON.formatData(dd[i].d,"DLC: ", "", COMMON.radix ))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.DLC,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.CRC_15_VAL) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.CRC_15_VAL, COMMON.formatData(dd[i].d,"CRC-15: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.CRC_15_VAL,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.CRC_17_VAL) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.CRC_17_VAL, COMMON.formatData(dd[i].d,"CRC-17: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.CRC_17_VAL,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.CRC_21_VAL) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.CRC_21_VAL, COMMON.formatData(dd[i].d,"CRC-21: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.CRC_21_VAL,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.SBC) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.SBC, COMMON.formatData(dd[i].d,"SBC: ", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.SBC,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.CRC_DELIMITER) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["Cd"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ACK_SLOT) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["A"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ACK_DELIMITER) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["Ad"])
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }
        }
    }

}(window.CAN = window.CAN || {}, jQuery));