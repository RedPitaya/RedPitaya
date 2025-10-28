class BinarySignalParser {
    constructor() {
        this.BinarySignalType = {
            UNDEFINED: 0,
            INT8: 1,
            INT16: 2,
            INT32: 3,
            UINT8: 4,
            UINT16: 5,
            UINT32: 6,
            FLOAT: 7,
            DOUBLE: 8
        };
    }

    /**
     * Parse binary buffer containing signal data
     * @param {ArrayBuffer} buffer - Binary data buffer
     * @returns {Array} Array of signal objects with name, dataType and data array
     */
    parse(buffer) {
        const results = {};
        let offset = 0;
        let dec_data = 0;
        const dataView = new DataView(buffer);

        while (offset + 4 <= buffer.byteLength) {
            // Read prefix (4 bytes)
            const prefix = String.fromCharCode(...new Uint8Array(buffer, offset, 4));

            // Validate prefix
            if (prefix !== 'NZIB' && prefix !== 'EZIB') {
                console.warn('Unknown prefix:', prefix);
                break;
            }

            // Check if header is available (12 bytes)
            if (offset + 16 > buffer.byteLength) break;

            // Read header
            const dataType = dataView.getUint32(offset + 4, true);
            const nameSize = dataView.getUint32(offset + 8 , true);
            const dataSize = dataView.getUint32(offset + 12 , true);
            const headerSize = dataView.getUint32(offset + 16 , true);


            // Check if name and data are available
            if (offset + headerSize + dataSize > buffer.byteLength) break;

            // Read signal name
            const name = String.fromCharCode(...new Uint8Array(buffer, offset + 20, nameSize));

            // Read raw data
            offset += headerSize;
            const rawData = new Uint8Array(buffer, offset, dataSize);
            offset += dataSize;

            // Process data based on compression
            let data;
            if (prefix === 'EZIB') {
                // Compressed data - decompress
                try {
                    const decompressed = pako.inflate(rawData);
                    dec_data += decompressed.byteLength
                    data = this.createTypedArray(new DataView(decompressed.buffer), 0, decompressed.byteLength, dataType);
                } catch (error) {
                    console.error('Decompression error for signal', name, error);
                    data = [];
                }
            } else {
                // Uncompressed data
                dec_data += dataSize
                data = this.createTypedArray(dataView, offset - dataSize, dataSize, dataType);
            }

            results[name] = {
                size:data.length,
                value:data
            };
        }
        return [results,dec_data];
    }

    /**
     * Create typed array from binary data based on data type
     * @param {DataView} dataView - Data view for reading binary data
     * @param {number} offset - Starting offset in buffer
     * @param {number} dataSize - Total size of data in bytes
     * @param {number} dataType - Signal data type from BinarySignalType
     * @returns {TypedArray} Typed array of corresponding type
     */
    createTypedArray(dataView, offset, dataSize, dataType) {
        const elementSize = this.getElementSize(dataType);

        // Validate data size alignment
        if (dataSize % elementSize !== 0) {
            console.warn(`Data size (${dataSize}) not aligned with element size (${elementSize}) for type ${dataType}`);
            return new Uint8Array(0);
        }

        const count = dataSize / elementSize;

        // Check bounds
        if (offset + dataSize > dataView.byteLength) {
            return new Uint8Array(0);
        }

        // Create appropriate typed array
        switch (dataType) {
            case this.BinarySignalType.INT8:
                return new Int8Array(dataView.buffer, offset, count);
            case this.BinarySignalType.INT16:
                return new Int16Array(dataView.buffer, offset, count);
            case this.BinarySignalType.INT32:
                return new Int32Array(dataView.buffer, offset, count);
            case this.BinarySignalType.UINT8:
                return new Uint8Array(dataView.buffer, offset, count);
            case this.BinarySignalType.UINT16:
                return new Uint16Array(dataView.buffer, offset, count);
            case this.BinarySignalType.UINT32:
                return new Uint32Array(dataView.buffer, offset, count);
            case this.BinarySignalType.FLOAT:
                return new Float32Array(dataView.buffer, offset, count);
            case this.BinarySignalType.DOUBLE:
                return new Float64Array(dataView.buffer, offset, count);
            default:
                console.warn(`Unknown data type: ${dataType}`);
                return new Uint8Array(0);
        }
    }

    /**
     * Get element size in bytes for given data type
     * @param {number} dataType - Signal data type
     * @returns {number} Size in bytes
     */
    getElementSize(dataType) {
        switch (dataType) {
            case this.BinarySignalType.INT8:
            case this.BinarySignalType.UINT8:
                return 1;
            case this.BinarySignalType.INT16:
            case this.BinarySignalType.UINT16:
                return 2;
            case this.BinarySignalType.INT32:
            case this.BinarySignalType.UINT32:
            case this.BinarySignalType.FLOAT:
                return 4;
            case this.BinarySignalType.DOUBLE:
                return 8;
            default:
                return 1;
        }
    }

    /**
     * Get type name from numeric data type
     * @param {number} dataType - Numeric data type
     * @returns {string} Type name
     */
    getTypeName(dataType) {
        for (const [name, value] of Object.entries(this.BinarySignalType)) {
            if (value === dataType) {
                return name;
            }
        }
        return 'UNKNOWN';
    }

    /**
     * Get typed array constructor name
     * @param {number} dataType - Numeric data type
     * @returns {string} Typed array constructor name
     */
    getTypedArrayName(dataType) {
        switch (dataType) {
            case this.BinarySignalType.INT8:
                return 'Int8Array';
            case this.BinarySignalType.INT16:
                return 'Int16Array';
            case this.BinarySignalType.INT32:
                return 'Int32Array';
            case this.BinarySignalType.UINT8:
                return 'Uint8Array';
            case this.BinarySignalType.UINT16:
                return 'Uint16Array';
            case this.BinarySignalType.UINT32:
                return 'Uint32Array';
            case this.BinarySignalType.FLOAT:
                return 'Float32Array';
            case this.BinarySignalType.DOUBLE:
                return 'Float64Array';
            default:
                return 'Uint8Array';
        }
    }

    /**
     * Pretty print signal information
     * @param {Array} signals - Array of signal objects
     */
    printSignals(signals) {
        signals.forEach(signal => {
            console.log("   Signal: ${signal.name}");
            console.log("   Type: ${this.getTypeName(signal.dataType)} (${signal.dataType})");
            console.log("   Data: [${signal.data.join(', ')}]");
            console.log("   Count: ${signal.data.length}");
            console.log('---');
        });
    }

    base64ToFloatArray(base64String) {
        // Decode the base64 string to a byte array
        const b64ToBuffer = (b64) => Uint8Array.from(atob(b64), c => c.charCodeAt(0)).buffer;
        let bytes = b64ToBuffer(base64String)
        // Create a Float32Array from the byte array
        const floatArray = new Float32Array(bytes.byteLength / 4);

        // Convert the byte array to a Float32Array
        for (let i = 0; i < floatArray.length; i++) {
            const byteIndex = i * 4;
            floatArray[i] = new DataView(bytes).getFloat32(byteIndex,true);
        }

        return floatArray;
    }

    base64ToDoubleArray(base64String) {
        // Decode the base64 string to a byte array
        const b64ToBuffer = (b64) => Uint8Array.from(atob(b64), c => c.charCodeAt(0)).buffer;
        let bytes = b64ToBuffer(base64String)
        // Create a Float32Array from the byte array
        const floatArray = new Float64Array(bytes.byteLength / 8);

        // Convert the byte array to a Float32Array
        for (let i = 0; i < floatArray.length; i++) {
            const byteIndex = i * 8;
            floatArray[i] = new DataView(bytes).getFloat64(byteIndex,true);
        }

        return floatArray;
    }

    base64ToIntArray(base64String) {
        // Decode the base64 string to a byte array
        if (base64String.length === 0) return new Int32Array(0)
        const b64ToBuffer = (b64) => Uint8Array.from(atob(b64), c => c.charCodeAt(0)).buffer;
        let bytes = b64ToBuffer(base64String)
        // Create a Float32Array from the byte array
        const intArray = new Int32Array(bytes.byteLength / 4);

        // Convert the byte array to a Float32Array
        for (let i = 0; i < intArray.length; i++) {
          const byteIndex = i * 4;
          intArray[i] = new DataView(bytes).getInt32(byteIndex,true);
        }

        return intArray;
    }


    base64ToByteArray(base64String) {
        // Decode the base64 string to a byte array
        const b64ToBuffer = (b64) => Uint8Array.from(atob(b64), c => c.charCodeAt(0)).buffer;
        let bytes = b64ToBuffer(base64String)
        // Create a Float32Array from the byte array
        const byteArray = new Uint8Array(bytes.byteLength);

        // Convert the byte array to a Float32Array
        for (let i = 0; i < byteArray.length; i++) {
          const byteIndex = i;
          byteArray[i] = new DataView(bytes).getUint8(byteIndex,true);
        }

        return byteArray;
    }


    checkPrefix(data, prefixString) {
        const prefix = new TextEncoder().encode(prefixString);
        if (data.length >= prefix.length) {
            const hasPrefix = prefix.every((byte, index) => data[index] === byte);
            if (hasPrefix) {
                return true;
            }
        }
        return false;
    }

    convert(d){
        let data = new Uint8Array(d)
        let compressed_data = data.length
        let isZip = this.checkPrefix(data,"EZIA")  || this.checkPrefix(data,"EZIB");
        let isBinary = this.checkPrefix(data,"EZIB") || this.checkPrefix(data,"NZIB") ;
        var receive = {}
        if (isBinary == false){
            data = data.slice(4)
            if (isZip)
                data = pako.inflate(data);
            var bytes = data
            var text = '';
            for(var i = 0; i < Math.ceil(bytes.length / 32768.0); i++) {
                text += String.fromCharCode.apply(null, bytes.slice(i * 32768, Math.min((i+1) * 32768, bytes.length)))
            }
            var receive = JSON.parse(text);
            receive["decompressed_data"] = text.length
        }else{
            data = this.parse(data.buffer)
            receive["signals"] = data[0]
            receive["decompressed_data"] = data[1]
        }
        receive["compressed_data"] = compressed_data
        for (const property in receive["signals"]) {
            if (receive["signals"][property]['type']){
                if (receive["signals"][property]['type'] == 'f'){
                    receive["signals"][property]['value'] = this.base64ToFloatArray(receive["signals"][property]['value'] )
                }
                if (receive["signals"][property]['type'] == 'd'){
                    receive["signals"][property]['value'] = this.base64ToDoubleArray(receive["signals"][property]['value'] )
                }
                if (receive["signals"][property]['type'] == 'i'){
                    receive["signals"][property]['value'] = this.base64ToIntArray(receive["signals"][property]['value'] )
                }
                if (receive["signals"][property]['type'] == 'h'){
                    receive["signals"][property]['value'] = this.base64ToByteArray(receive["signals"][property]['value'] )
                }
            }
        }
        return receive
    }
}