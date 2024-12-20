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
		CRC_VAL			: 23, // CRC Value
		FSB				: 24, // fixed stuff-bit (FSB)
		SBC				: 25, // Stuff bits before CRC in FD mode

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
        SRR:"#cfccc8",
        RESERV_BIT:"#cfccc8",
        RESERV_BIT_FLEX:"#cfccc8",
        IDE:"#b5c4b5",
        DLC:"#a82895",
        CRC_VAL: "#d1323f",
        SBC: "#a87b28",
        CRC_DELIMITER:"#cfccc8",
        ACK_SLOT:"#cfccc8",
        ACK_DELIMITER:"#cfccc8",
    }

    CAN.drawDecoded = function(plot, canvascontext, offset, decoded_data) {


        var dd = decoded_data;


        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {
            if (dd[i].c == CAN.ANNOTATIONS.START_OF_FRAME) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.START_OF_FRAME, "S");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.START_OF_FRAME,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.END_OF_FRAME) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.END_OF_FRAME, COMMON.formatData(dd[i].d, "End: "))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.END_OF_FRAME,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.PAYLOAD_DATA) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.PAYLOAD_DATA, COMMON.formatData(dd[i].d))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.PAYLOAD_DATA,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ID) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.ID, COMMON.formatData(dd[i].d, "ID: "))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.ID,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.EXT_ID) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.ID, COMMON.formatData(dd[i].d, "Ext ID: "))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.ID,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.STUFF_BIT) {
                COMMON.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.STUFF_BIT, ["Stuff"])
                COMMON.drawBitsBarsTop(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.STUFF_BIT,dd[i].b, false)
            }

            if (dd[i].c == CAN.ANNOTATIONS.FSB) {
                COMMON.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.FSB, ["FSB"])
                COMMON.drawBitsBarsTop(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.FSB, dd[i].b, false)
            }

            if (dd[i].c == CAN.ANNOTATIONS.RTR) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RTR, ["RTR"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RTR,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.SRR) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.SRR, ["SRR"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.SRR,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.RESERV_BIT) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["Rs"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.IDE) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.IDE, ["IDE"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.IDE,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.RESERV_BIT_FLEX) {
                COMMON.drawTextTop(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.STUFF_BIT, ["FD mode"])
            }

            if (dd[i].c == CAN.ANNOTATIONS.DLC) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.DLC, COMMON.formatData(dd[i].d,"DLC: "))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.DLC,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.CRC_VAL) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.CRC_VAL, COMMON.formatData(dd[i].d,"CRC: "))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.CRC_VAL,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.SBC) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, CAN.colors.SBC, COMMON.formatData(dd[i].d,"SBC: "))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.SBC,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.CRC_DELIMITER) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["Cd"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ACK_SLOT) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["A"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }

            if (dd[i].c == CAN.ANNOTATIONS.ACK_DELIMITER) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT, ["Ad"])
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,CAN.colors.RESERV_BIT,dd[i].b)
            }
        }
            // if (dd[i].c == UART.ANNOTATIONS.STOP_BIT) {
            //     COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.STOP, "O");
            //     COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.STOP,dd[i].b)
            // }

            // if (dd[i].c == UART.ANNOTATIONS.STOP_BIT_ERR) {
            //     COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "O!");
            //     COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            // }

            // if (dd[i].c == UART.ANNOTATIONS.PARITY_BIT) {
            //     COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.PARITY, "P");
            //     COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.PARITY,dd[i].b)
            // }

            // if (dd[i].c == UART.ANNOTATIONS.PARITY_ERR) {
            //     COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "P!");
            //     COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            // }



        // var dd = decoded_data.value;
        // var last_pos = -OSC.offsetForDecoded;
        // var coef = OSC.time_scale;
        // COMMON.appendHead("Time[s], Bus, R/W, Address/Data, Data, ACK/NAK");
        // var tmpAckNack = "";
        // var tmpLastPos = 0;

        // for (var i = 0; i < dd.length; i++) {
        //     if ((dd[i].control != CAN.ANNOTATIONS.BIT) && (dd[i].control != CAN.ANNOTATIONS.NOTHING)) {
        //         var color = CAN.colors[dd[i].control];
        //         var textVal = "";
        //         switch (dd[i].control) {

        //             case CAN.ANNOTATIONS.START_OF_FRAME:
        //                 textVal = "S";
        //                 COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, textVal);
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN, Start of frame");
        //                 tmpLastPos = last_pos + 10; // 10 - size of circle
        //                 break;
        //             case CAN.ANNOTATIONS.ID:
        //                 textVal = "ID: " + dd[i].data;
        //                 COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "ID: "));
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "ID: ")[0]);
        //                 tmpLastPos = last_pos + 10; // 10 - size of circle
        //                 break;
        //             case CAN.ANNOTATIONS.END_OF_FRAME:
        //                 textVal = "E";
        //                 COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, textVal);
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN, End of frame");
        //                 break;
        //             case CAN.ANNOTATIONS.EXT_ID:
        //                 textVal = "EXT ID: " + dd[i].data;
        //                 COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "EXT ID: ")[0]);
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "EXT ID: ")[0]);
        //                 break;
        //             case CAN.ANNOTATIONS.FULL_ID:
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "FULL ID: ")[0]);
        //                 break;
        //             case CAN.ANNOTATIONS.IDE:
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "IDE: ")[0]);
        //                 break;
        //             case CAN.ANNOTATIONS.RESERV_BIT:
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "RESERV_BIT: ")[0]);
        //                 break;
        //             case CAN.ANNOTATIONS.RTR:
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "RTR: ")[0]);
        //                 break;
        //             case CAN.ANNOTATIONS.SRR:
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "SRR: ")[0]);
        //                 break;
        //             case CAN.ANNOTATIONS.STUFF_BIT:
        //                 COMMON.drawTopCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, "SB");
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "STUFF_BIT: ")[0]);
        //                 last_pos += dd[i].length * coef;
        //                 break;
        //             case CAN.ANNOTATIONS.DLC:
        //                 COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, "D");
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "DLC: ")[0]);
        //                 tmpLastPos = last_pos + 10; // 10 - size of circle
        //                 break;
        //             case CAN.ANNOTATIONS.PAYLOAD_DATA:
        //                 COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "DATA: "));
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "PAYLOAD_DATA: ")[0]);
        //                 tmpLastPos = last_pos + 10;
        //                 break;
        //             case CAN.ANNOTATIONS.CRC_LEN:
        //                 textVal = "CRC TYPE: " + dd[i].data;
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.CRC_DELIMITER:
        //                 textVal = "CRC DELIMITER: " + dd[i].data;
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.ACK_DELIMITER:
        //                 textVal = "ACK DELIMITE: " + dd[i].data;
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.ACK_SLOT:
        //                 textVal = "ACK SLOT: " + dd[i].data;
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.ESI:
        //                 textVal = "ESI: " + dd[i].data;
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.RESERV_BIT_FLEX:
        //                 textVal = "Flexible data: " + dd[i].data;
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.CRC_SEQ:
        //                 COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "CRC: "));
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "CRC: ")[0]);
        //                 tmpLastPos = last_pos + 10;
        //                 break;
        //             case CAN.ANNOTATIONS.ERROR_1:
        //                 textVal = "[E] Start of frame (SOF) must be a dominant bit";
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.ERROR_2:
        //                 textVal = "[E] Data length code (DLC) > 8 is not allowed";
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.ERROR_3:
        //                 textVal = "[E] End of frame (EOF) must be 7 recessive bits";
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.WARNING_1:
        //                 textVal = "[W]Identifier bits 10..4 must not be all recessive";
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.WARNING_2:
        //                 textVal = "[W] CRC delimiter must be a recessive bit";
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.WARNING_3:
        //                 textVal = "[W] ACK delimiter must be a recessive bit";
        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
        //                 break;
        //             case CAN.ANNOTATIONS.SYNC:

        //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "Sync: ")[0]);
        //                 break;
        //             default:
        //                 break;
        //         }
        //     } else {
        //         last_pos += dd[i].length * coef;
        //     }
        // }
    }

}(window.CAN = window.CAN || {}, jQuery));