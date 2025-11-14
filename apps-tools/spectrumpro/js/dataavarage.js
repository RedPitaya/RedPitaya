/*
 * Red Pitaya Spectrum Analizator client
 * Data averager class for processing multi-channel, multi-mode data
 * with configurable time-based averaging window
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */
/**
 * Data averager class for processing multi-channel, multi-mode data
 * with configurable time-based averaging window and per-channel processing
 */
/**
 * Data averager class for processing multi-channel, multi-mode data
 * with configurable time-based averaging window and per-channel processing
 */
/**
 * Data averager class for processing multi-channel, multi-mode data
 * with configurable time-based averaging window and per-channel processing
 * Supports two synchronized data arrays
 */

class DataAverager {
    /**
     * Create a DataAverager instance
     * @param {number} channels - Number of channels (X)
     * @param {number} modes - Number of modes (M)
     * @param {number} maxWindowSize - Maximum window size for averaging
     * @param {number} targetTimeWindow - Target time window in milliseconds
     * @param {number} dataArrays - Number of data arrays to support (1 or 2)
     */
    constructor(channels, modes, maxWindowSize = 100, targetTimeWindow = 1000, dataArrays = 1) {
        this.channels = channels;
        this.modes = modes;
        this.maxWindowSize = maxWindowSize;
        this.targetTimeWindow = targetTimeWindow;
        this.dataArrays = Math.min(Math.max(1, dataArrays), 2); // Support 1 or 2 arrays

        // Data storage: arrayIndex × channels × modes × window
        this.data = this._createEmptyStorage();

        // Shared timestamps for synchronized processing (one timestamp per sample for both arrays)
        this.timestamps = [];

        // Current average values for each array
        this.currentAverages = this._createEmptyAverages();

        // Channel enabled/disabled state (shared for both arrays)
        this.enabledChannels = new Array(channels).fill(true);

        // Array enabled/disabled state
        this.enabledArrays = new Array(this.dataArrays).fill(true);
    }

    /**
     * Create empty data storage structure
     * @private
     */
    _createEmptyStorage() {
        const storage = [];
        for (let arrayIndex = 0; arrayIndex < this.dataArrays; arrayIndex++) {
            storage[arrayIndex] = [];
            for (let channel = 0; channel < this.channels; channel++) {
                storage[arrayIndex][channel] = [];
                for (let mode = 0; mode < this.modes; mode++) {
                    storage[arrayIndex][channel][mode] = [];
                }
            }
        }
        return storage;
    }

    /**
     * Create empty averages structure
     * @private
     */
    _createEmptyAverages() {
        const averages = [];
        for (let arrayIndex = 0; arrayIndex < this.dataArrays; arrayIndex++) {
            averages[arrayIndex] = [];
            for (let channel = 0; channel < this.channels; channel++) {
                averages[arrayIndex][channel] = new Float32Array(this.modes);
            }
        }
        return averages;
    }

    /**
     * Add synchronized data for all arrays
     * @param {Array<Float32Array>} dataArrays - Array of Float32Arrays [array0Data, array1Data]
     * @param {number} timestamp - Timestamp (Date.now())
     */
    addSynchronizedData(dataArrays, timestamp = Date.now()) {
        // Validate input
        if (!Array.isArray(dataArrays) || dataArrays.length !== this.dataArrays) {
            console.error(`Expected array of ${this.dataArrays} Float32Arrays`);
            return;
        }

        // Validate all data arrays
        for (let i = 0; i < this.dataArrays; i++) {
            if (!this._validateFullData(dataArrays[i])) {
                console.error(`Invalid data format for array ${i}: expected Float32Array of length`, this.channels * this.modes);
                return;
            }
            if (!this.enabledArrays[i]) {
                console.warn(`Array ${i} is disabled, data not processed`);
                return;
            }
        }

        // Add shared timestamp
        this.timestamps.push(timestamp);

        // Process data for all arrays and channels
        for (let arrayIndex = 0; arrayIndex < this.dataArrays; arrayIndex++) {
            if (this.enabledArrays[arrayIndex]) {
                for (let channel = 0; channel < this.channels; channel++) {
                    if (this.enabledChannels[channel]) {
                        this._addChannelData(channel, dataArrays[arrayIndex], arrayIndex);
                    }
                }
            }
        }

        // Limit timestamps array size
        if (this.timestamps.length > this.maxWindowSize) {
            this.timestamps.shift();
        }

        // Update averages for all enabled arrays and channels
        this._updateAllAverages();
    }

    /**
     * Add synchronized data for specific channel from all arrays
     * @param {number} channel - Channel index (0-based)
     * @param {Array<Float32Array>} dataArrays - Array of Float32Arrays [array0Data, array1Data]
     * @param {number} timestamp - Timestamp (Date.now())
     */
    addSynchronizedChannelData(channel, dataArrays, timestamp = Date.now()) {
        // Validate input
        if (channel < 0 || channel >= this.channels) {
            console.error(`Invalid channel index: ${channel}`);
            return;
        }

        if (!Array.isArray(dataArrays) || dataArrays.length !== this.dataArrays) {
            console.error(`Expected array of ${this.dataArrays} Float32Arrays`);
            return;
        }

        // Validate all data arrays
        for (let i = 0; i < this.dataArrays; i++) {
            if (!this._validateFullData(dataArrays[i])) {
                console.error(`Invalid data format for array ${i}: expected Float32Array of length`, this.channels * this.modes);
                return;
            }
            if (!this.enabledArrays[i]) {
                console.warn(`Array ${i} is disabled, data not processed`);
                return;
            }
        }

        if (!this.enabledChannels[channel]) {
            console.warn(`Channel ${channel} is disabled, data not processed`);
            return;
        }

        // Add shared timestamp
        this.timestamps.push(timestamp);

        // Process data for all arrays for this specific channel
        for (let arrayIndex = 0; arrayIndex < this.dataArrays; arrayIndex++) {
            if (this.enabledArrays[arrayIndex]) {
                this._addChannelData(channel, dataArrays[arrayIndex], arrayIndex);
            }
        }

        // Limit timestamps array size
        if (this.timestamps.length > this.maxWindowSize) {
            this.timestamps.shift();
        }

        // Update averages for this channel in all arrays
        this._updateChannelAverages(channel);
    }

    /**
     * Internal method to add data for a specific channel from full dataset
     * @private
     */
    _addChannelData(channel, fullData, arrayIndex) {
        // Calculate starting index in flat array for this channel
        const startIndex = channel * this.modes;

        // Add data for each mode in the channel for this array
        for (let mode = 0; mode < this.modes; mode++) {
            const value = fullData[startIndex + mode];
            this.data[arrayIndex][channel][mode].push(value);

            // Limit window size
            if (this.data[arrayIndex][channel][mode].length > this.maxWindowSize) {
                this.data[arrayIndex][channel][mode].shift();
            }
        }
    }

    /**
     * Validate array index
     * @private
     */
    _validateArrayIndex(arrayIndex) {
        if (arrayIndex < 0 || arrayIndex >= this.dataArrays) {
            console.error(`Invalid array index: ${arrayIndex}. Supported: 0 to ${this.dataArrays - 1}`);
            return false;
        }
        return true;
    }

    /**
     * Validate full dataset (all channels)
     * @private
     */
    _validateFullData(data) {
        if (!(data instanceof Float32Array)) {
            return false;
        }

        if (data.length !== this.channels * this.modes) {
            return false;
        }

        for (let i = 0; i < data.length; i++) {
            if (!isFinite(data[i])) {
                return false;
            }
        }

        return true;
    }

    /**
     * Update averages for all enabled channels in all arrays
     * @private
     */
    _updateAllAverages() {
        for (let channel = 0; channel < this.channels; channel++) {
            if (this.enabledChannels[channel]) {
                this._updateChannelAverages(channel);
            }
        }
    }

    /**
     * Update averages for specific channel in all arrays
     * @private
     */
    _updateChannelAverages(channel) {
        if (this.timestamps.length === 0) return;

        const currentTime = this.timestamps[this.timestamps.length - 1];
        const oldestTime = this.timestamps[0];
        const actualTimeWindow = currentTime - oldestTime;

        const optimalWindowSize = this._calculateOptimalWindowSize(actualTimeWindow);

        // Update averages for this channel in all arrays
        for (let arrayIndex = 0; arrayIndex < this.dataArrays; arrayIndex++) {
            if (this.enabledArrays[arrayIndex]) {
                this._calculateChannelAverages(channel, arrayIndex, optimalWindowSize);
            }
        }
    }

    /**
     * Calculate optimal window size for current time window
     * @private
     */
    _calculateOptimalWindowSize(actualTimeWindow) {
        if (actualTimeWindow <= this.targetTimeWindow) {
            return Math.min(this.timestamps.length, this.maxWindowSize);
        } else {
            let optimalSize = 1;

            for (let i = this.timestamps.length - 1; i >= 0; i--) {
                const timeDiff = this.timestamps[this.timestamps.length - 1] - this.timestamps[i];
                if (timeDiff <= this.targetTimeWindow) {
                    optimalSize = this.timestamps.length - i;
                    break;
                }
            }

            return Math.min(optimalSize, this.maxWindowSize);
        }
    }

    /**
     * Calculate average values for specific channel in specific array
     * @private
     */
    _calculateChannelAverages(channel, arrayIndex, windowSize) {
        const startIndex = Math.max(0, this.timestamps.length - windowSize);

        for (let mode = 0; mode < this.modes; mode++) {
            const channelData = this.data[arrayIndex][channel][mode];
            const relevantData = channelData.slice(startIndex);

            if (relevantData.length === 0) {
                this.currentAverages[arrayIndex][channel][mode] = 0;
                continue;
            }

            const sum = relevantData.reduce((acc, val) => acc + val, 0);
            this.currentAverages[arrayIndex][channel][mode] = sum / relevantData.length;
        }
    }

    /**
     * Enable or disable specific channel (affects both arrays)
     * @param {number} channel - Channel index
     * @param {boolean} enabled - Enable state
     */
    setChannelEnabled(channel, enabled) {
        if (channel >= 0 && channel < this.channels) {
            this.enabledChannels[channel] = enabled;
            if (enabled) {
                this._updateChannelAverages(channel);
            }
        }
    }

    /**
     * Enable or disable specific array
     * @param {number} arrayIndex - Array index (0 or 1)
     * @param {boolean} enabled - Enable state
     */
    setArrayEnabled(arrayIndex, enabled) {
        if (this._validateArrayIndex(arrayIndex)) {
            this.enabledArrays[arrayIndex] = enabled;
            if (enabled) {
                this._updateAllAverages();
            }
        }
    }

    /**
     * Get enabled state of specific channel
     * @param {number} channel - Channel index
     * @returns {boolean} Enabled state
     */
    isChannelEnabled(channel) {
        return channel >= 0 && channel < this.channels ? this.enabledChannels[channel] : false;
    }

    /**
     * Get enabled state of specific array
     * @param {number} arrayIndex - Array index
     * @returns {boolean} Enabled state
     */
    isArrayEnabled(arrayIndex) {
        return this._validateArrayIndex(arrayIndex) ? this.enabledArrays[arrayIndex] : false;
    }

    /**
     * Get current average values as Float32Array for specific array
     * @param {number} arrayIndex - Array index (0 or 1)
     * @returns {Float32Array} Flat array of averaged values (X * M)
     */
    getAverages(arrayIndex = 0) {
        if (!this._validateArrayIndex(arrayIndex)) {
            return new Float32Array(this.channels * this.modes);
        }

        const result = new Float32Array(this.channels * this.modes);
        let index = 0;

        for (let channel = 0; channel < this.channels; channel++) {
            for (let mode = 0; mode < this.modes; mode++) {
                result[index++] = (this.enabledChannels[channel] && this.enabledArrays[arrayIndex]) ?
                    this.currentAverages[arrayIndex][channel][mode] : 0;
            }
        }

        return result;
    }

    /**
     * Get averages for all arrays combined (only for 2 arrays)
     * @returns {Float32Array} Combined averages from both arrays
     */
    getCombinedAverages() {
        if (this.dataArrays !== 2) {
            console.warn('Combined averages only available for 2 arrays');
            return this.getAverages(0);
        }

        const result = new Float32Array(this.channels * this.modes);
        let index = 0;

        for (let channel = 0; channel < this.channels; channel++) {
            for (let mode = 0; mode < this.modes; mode++) {
                let sum = 0;
                let count = 0;

                if (this.enabledChannels[channel]) {
                    if (this.enabledArrays[0]) {
                        sum += this.currentAverages[0][channel][mode];
                        count++;
                    }
                    if (this.enabledArrays[1]) {
                        sum += this.currentAverages[1][channel][mode];
                        count++;
                    }
                }

                result[index++] = count > 0 ? sum / count : 0;
            }
        }

        return result;
    }

    /**
     * Get averages for specific channel only in specific array
     * @param {number} channel - Channel index
     * @param {number} arrayIndex - Array index (0 or 1)
     * @returns {Float32Array} Averages for the channel (length M)
     */
    getChannelAverages(channel, arrayIndex = 0) {
        if (!this._validateArrayIndex(arrayIndex) || channel < 0 || channel >= this.channels) {
            return new Float32Array(this.modes);
        }

        if (!this.enabledChannels[channel] || !this.enabledArrays[arrayIndex]) {
            return new Float32Array(this.modes);
        }

        return new Float32Array(this.currentAverages[arrayIndex][channel]);
    }

    /**
     * Get current 2D average values for specific array
     * @param {number} arrayIndex - Array index (0 or 1)
     * @returns {Array} 2D array of averages [channel][mode]
     */
    getAverages2D(arrayIndex = 0) {
        if (!this._validateArrayIndex(arrayIndex)) {
            return Array(this.channels).fill().map(() => new Array(this.modes).fill(0));
        }

        return this.currentAverages[arrayIndex].map((channelAverages, channel) =>
            (this.enabledChannels[channel] && this.enabledArrays[arrayIndex]) ?
                Array.from(channelAverages) :
                new Array(this.modes).fill(0)
        );
    }

    /**
     * Get current effective window size (shared for both arrays)
     * @returns {number} Current window size
     */
    getWindowSize() {
        if (this.timestamps.length === 0) return 0;

        const currentTime = Date.now();
        const oldestTime = this.timestamps[0];
        const actualTimeWindow = currentTime - oldestTime;

        return this._calculateOptimalWindowSize(actualTimeWindow);
    }

    /**
     * Get statistics about current data state
     * @returns {Object} Statistics object
     */
    getStats() {
        const enabledChannels = this.enabledChannels.filter(enabled => enabled).length;
        const enabledArrays = this.enabledArrays.filter(enabled => enabled).length;
        const windowSize = this.getWindowSize();

        return {
            totalChannels: this.channels,
            enabledChannels: enabledChannels,
            totalArrays: this.dataArrays,
            enabledArrays: enabledArrays,
            modes: this.modes,
            windowSize: windowSize,
            dataPoints: this.timestamps.length,
            maxWindowSize: this.maxWindowSize,
            targetTimeWindow: this.targetTimeWindow,
            actualTimeWindow: this.timestamps.length > 0 ?
                Date.now() - this.timestamps[0] : 0
        };
    }

    /**
     * Clear all stored data (both arrays)
     */
    clear() {
        this.data = this._createEmptyStorage();
        this.timestamps = [];
        this.currentAverages = this._createEmptyAverages();
    }

    /**
     * Update averaging settings for all arrays
     * @param {number} maxWindowSize - New maximum window size
     * @param {number} targetTimeWindow - New target time window in ms
     */
    updateSettings(maxWindowSize, targetTimeWindow) {
        this.maxWindowSize = maxWindowSize;
        this.targetTimeWindow = targetTimeWindow;
        this._updateAllAverages();
    }
}

// Example usage:
/*
// Create averager for 4 channels, 2 modes, supporting 2 synchronized data arrays
const averager = new DataAverager(4, 2, 100, 1500, 2);

// Synchronized data for both arrays (must be added together)
const array0Data = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]);
const array1Data = new Float32Array([10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0]);

// Add synchronized data for both arrays
averager.addSynchronizedData([array0Data, array1Data]);

// Add synchronized data for specific channel only
averager.addSynchronizedChannelData(2, [array0Data, array1Data]);

// Get averages for individual arrays
const averages0 = averager.getAverages(0);
const averages1 = averager.getAverages(1);

// Get combined averages from both arrays
const combined = averager.getCombinedAverages();

// Get statistics
const stats = averager.getStats();
console.log('Stats:', stats);
*/