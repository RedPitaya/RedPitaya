(function(RP_DATA_STREAM, $, undefined) {

    // App configuration
    RP_DATA_STREAM.config = {};
    RP_DATA_STREAM.config.server_ip = ''; // Leave empty on production, it is used for testing only
    RP_DATA_STREAM.config.socket_url = 'ws://' + (RP_DATA_STREAM.config.server_ip.length ? RP_DATA_STREAM.config.server_ip : window.location.hostname) + ':9900';
    RP_DATA_STREAM.config.debug = false

    RP_DATA_STREAM.socket_opened = false

    // Other global variables
    RP_DATA_STREAM.ws = null;


    RP_DATA_STREAM.client_log = function(...args) {
        if (RP_DATA_STREAM.config.debug){
            const d = new Date();
            console.log("LOG:RP_DATA_STREAM.js",d.getHours() + ":" + d.getMinutes() + ":"+ d.getSeconds() + ":" + d.getMilliseconds() ,...args);
        }
    }


    class BufferedDataStreamParser {
        constructor() {
            this.state = 'waitingForHeader';
            this.currentHeader = null;
            this.completePackets = [];
        }

        parseMessage(binaryData) {
            const dataView = new DataView(binaryData);
            OSC.compressed_data += dataView.byteLength
            OSC.decompressed_data += dataView.byteLength

            switch (this.state) {
                case 'waitingForHeader':
                    if (binaryData.byteLength === 24) {
                        const header = {
                            magic: dataView.getUint32(0, true),
                            dataSize: dataView.getUint32(4, true),
                            elementCount: dataView.getUint32(8, true),
                            channel: dataView.getUint32(12, true),
                            decimation: dataView.getUint32(16, true),
                            tScale: dataView.getFloat32(20,true)
                        };

                        if (header.magic !== 0xAABBCCDD) {
                            throw new Error('Invalid magic number');
                        }

                        this.currentHeader = header;
                        this.state = 'waitingForData';
                        return null;
                    }
                    break;

                case 'waitingForData':
                    if (binaryData.byteLength === this.currentHeader.dataSize) {
                        const floatData = [];
                        for (let i = 0; i < this.currentHeader.elementCount; i++) {
                            floatData.push(dataView.getFloat32(i * 4, true));
                        }

                        const completePacket = {
                            header: this.currentHeader,
                            data: floatData,
                            channel: this.currentHeader.channel,
                            timestamp: Date.now(),
                            decimation: this.currentHeader.decimation,
                            tScale: this.currentHeader.tScale
                        };

                        this.completePackets.push(completePacket);
                        this.state = 'waitingForHeader';
                        this.currentHeader = null;
                        return completePacket;
                    } else {
                        throw new Error(`Expected ${this.currentHeader.dataSize} bytes, got ${binaryData.byteLength}`);
                    }
                    break;
            }

            return null;
        }

        getNextPacket() {
            return this.completePackets.shift();
        }

        hasCompletePackets() {
            return this.completePackets.length > 0;
        }

        reset() {
            this.state = 'waitingForHeader';
            this.currentHeader = null;
            this.completePackets = [];
        }
    }

    // Creates a WebSocket connection with the web server
    RP_DATA_STREAM.connectWebSocket = function() {

        const bufferedParser = new BufferedDataStreamParser();

        if (window.WebSocket) {
            RP_DATA_STREAM.ws = new WebSocket(RP_DATA_STREAM.config.socket_url);
            RP_DATA_STREAM.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            RP_DATA_STREAM.ws = new MozWebSocket(RP_DATA_STREAM.config.socket_url);
            RP_DATA_STREAM.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (RP_DATA_STREAM.ws) {
            RP_DATA_STREAM.ws.onopen = function() {
                RP_DATA_STREAM.client_log('Socket opened');
            };

            RP_DATA_STREAM.ws.onclose = function() {
                RP_DATA_STREAM.socket_opened = false;
                RP_DATA_STREAM.client_log('Socket closed');
            };

            RP_DATA_STREAM.ws.onerror = function(ev) {
                if (!RP_DATA_STREAM.socket_opened)
                    setTimeout(RP_DATA_STREAM.connectWebSocket, 2000);
                RP_DATA_STREAM.client_log('Websocket error: ', ev);
            };

            RP_DATA_STREAM.ws.onmessage = function(ev) {
                try {

                    bufferedParser.parseMessage(ev.data);

                    while (bufferedParser.hasCompletePackets()) {
                        const packet = bufferedParser.getNextPacket();
                        let c = "CH" + (packet.channel + 1)
                        if (OSC.taMode[c])
                            OSC.taMode[c].setPoints(packet.data,packet.decimation,packet.tScale)
                    }

                } catch (e) {
                    console.log(e);
                    bufferedParser.reset()
                }
            };
        }
    };

}(window.RP_DATA_STREAM = window.RP_DATA_STREAM || {}, jQuery));


// Page onload event handler
$(function() {
    $(window).on('beforeunload', function() {
        RP_DATA_STREAM.ws.onclose = function() {}; // disable onclose handler first
        RP_DATA_STREAM.ws.close();
    });
})
