(function(CAN, $, undefined) {
    CAN.ANNOTATIONS = {
        PAYLOAD_DATA: 0, // 'Payload data'
        START_OF_FRAME: 1, // 'Start of frame'
        END_OF_FRAME: 2, // 'End of frame'
        ID: 3, // 'Identifier'
        EXT_ID: 4, // 'Extended identifier'
        FULL_ID: 5, // 'Full identifier'
        IDE: 6, // 'Identifier extension bit'
        RESERV_BIT: 7, // 'Reserved bit 0 and 1'
        RTR: 8, // 'Remote transmission request'
        SRR: 9, // 'Substitute remote request'
        DLC: 10, // 'Data length count'
        CRC_SEQ: 11, // 'CRC sequence'
        CRC_DELIMITER: 12, // 'CRC delimiter'
        ACK_SLOT: 13, // 'ACK slot'
        ACK_DELIMITER: 14, // 'ACK delimiter'
        STUFF_BIT: 15, // 'Stuff bit'
        WARNING: 16, // 'Warning unknow'
        BIT: 17, // 'Bit'
        ERROR_1: 18, // 'Start of frame (SOF) must be a dominant bit'
        ERROR_2: 19, // 'Data length code (DLC) > 8 is not allowed'
        ERROR_3: 20, // 'End of frame (EOF) must be 7 recessive bits'
        WARNING_1: 21, // 'Identifier bits 10..4 must not be all recessive'
        WARNING_2: 22, // 'CRC delimiter must be a recessive bit'
        WARNING_3: 23, // 'ACK delimiter must be a recessive bit'
        BRS: 24, // 'Bit rate switch'
        ESI: 25, // 'Error state indicator'
        CRC_LEN: 26, // 'Crc type
        RESERV_BIT_FLEX: 27, // 'Flexible data'
        NOTHING: 28,
        SYNC: 29

    };

    CAN.data_arr = [];

    CAN.colors = [
        "#243cf0", //
        "#243cf0", // 
        "#243cf0", // 
        "#243cf0", // 
        "#243cf0", //
        "#243cf0", //
        "#154473", //
        "#154473", //
        "#154473", //
        "#154473", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#ff3cf0", // STUFF_BIT
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
        "#243cf0", //
    ];

    CAN.drawDecoded = function(index, plot, canvascontext, series, decoded_data, format) {
        var dd = decoded_data.value;
        var last_pos = -OSC.offsetForDecoded;
        var coef = OSC.time_scale;
        COMMON.appendHead("Time[s], Bus, R/W, Address/Data, Data, ACK/NAK");
        var tmpAckNack = "";
        var tmpLastPos = 0;

        for (var i = 0; i < dd.length; i++) {
            if ((dd[i].control != CAN.ANNOTATIONS.BIT) && (dd[i].control != CAN.ANNOTATIONS.NOTHING)) {
                var color = CAN.colors[dd[i].control];
                var textVal = "";
                switch (dd[i].control) {

                    case CAN.ANNOTATIONS.START_OF_FRAME:
                        textVal = "S";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN, Start of frame");
                        tmpLastPos = last_pos + 10; // 10 - size of circle
                        break;
                    case CAN.ANNOTATIONS.ID:
                        textVal = "ID: " + dd[i].data;
                        COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "ID: "));
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "ID: ")[0]);
                        tmpLastPos = last_pos + 10; // 10 - size of circle
                        break;
                    case CAN.ANNOTATIONS.END_OF_FRAME:
                        textVal = "E";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN, End of frame");
                        break;
                    case CAN.ANNOTATIONS.EXT_ID:
                        textVal = "EXT ID: " + dd[i].data;
                        COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "EXT ID: ")[0]);
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "EXT ID: ")[0]);
                        break;
                    case CAN.ANNOTATIONS.FULL_ID:
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "FULL ID: ")[0]);
                        break;
                    case CAN.ANNOTATIONS.IDE:
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "IDE: ")[0]);
                        break;
                    case CAN.ANNOTATIONS.RESERV_BIT:
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "RESERV_BIT: ")[0]);
                        break;
                    case CAN.ANNOTATIONS.RTR:
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "RTR: ")[0]);
                        break;
                    case CAN.ANNOTATIONS.SRR:
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "SRR: ")[0]);
                        break;
                    case CAN.ANNOTATIONS.STUFF_BIT:
                        COMMON.drawTopCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, "SB");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "STUFF_BIT: ")[0]);
                        last_pos += dd[i].length * coef;
                        break;
                    case CAN.ANNOTATIONS.DLC:
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length) * coef, color, "D");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "DLC: ")[0]);
                        tmpLastPos = last_pos + 10; // 10 - size of circle
                        break;
                    case CAN.ANNOTATIONS.PAYLOAD_DATA:
                        COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "DATA: "));
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "PAYLOAD_DATA: ")[0]);
                        tmpLastPos = last_pos + 10;
                        break;
                    case CAN.ANNOTATIONS.CRC_LEN:
                        textVal = "CRC TYPE: " + dd[i].data;
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.CRC_DELIMITER:
                        textVal = "CRC DELIMITER: " + dd[i].data;
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.ACK_DELIMITER:
                        textVal = "ACK DELIMITE: " + dd[i].data;
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.ACK_SLOT:
                        textVal = "ACK SLOT: " + dd[i].data;
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.ESI:
                        textVal = "ESI: " + dd[i].data;
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.RESERV_BIT_FLEX:
                        textVal = "Flexible data: " + dd[i].data;
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.CRC_SEQ:
                        COMMON.drawHexagon(index, plot, canvascontext, series, tmpLastPos, last_pos - tmpLastPos, color, COMMON.formatData(dd[i].data, "CRC: "));
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, "CAN," + COMMON.formatData(dd[i].data, "CRC: ")[0]);
                        tmpLastPos = last_pos + 10;
                        break;
                    case CAN.ANNOTATIONS.ERROR_1:
                        textVal = "[E] Start of frame (SOF) must be a dominant bit";
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.ERROR_2:
                        textVal = "[E] Data length code (DLC) > 8 is not allowed";
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.ERROR_3:
                        textVal = "[E] End of frame (EOF) must be 7 recessive bits";
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.WARNING_1:
                        textVal = "[W]Identifier bits 10..4 must not be all recessive";
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.WARNING_2:
                        textVal = "[W] CRC delimiter must be a recessive bit";
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.WARNING_3:
                        textVal = "[W] ACK delimiter must be a recessive bit";
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + textVal);
                        break;
                    case CAN.ANNOTATIONS.SYNC:

                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "CAN," + COMMON.formatData(dd[i].data, "Sync: ")[0]);
                        break;
                    default:
                        break;
                }
            } else {
                last_pos += dd[i].length * coef;
            }
        }
    }

}(window.CAN = window.CAN || {}, jQuery));