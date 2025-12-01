/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

class MinMaxController {
    constructor() {
        this.mode_channel = {};
        this.min = {};
        this.max = {};
        this.min_value = {};
        this.max_value = {};

    }

    destroy() {
        this.width = undefined;
    }


    processing(channel, data, mode) {
        let ch_min = channel + "_min"
        let ch_max = channel + "_max"
        if (this.mode_channel[channel] && this.mode_channel[channel] != mode){
            this.resetByChannel(channel)
        }

        if ((this.min[ch_min] && this.min[ch_min].length != data.length) || (this.max[ch_max] && this.max[ch_max].length != data.length)){
            this.resetByChannel(channel)
        }

        this.mode_channel[channel] = mode
        if (this.min[ch_min] == undefined){
            this.min[ch_min] = new Float32Array(data)
        }else{
            for(let i = 0; i < data.length; i++){
                if (this.min[ch_min][i] > data[i]){
                    this.min[ch_min][i] = data[i]
                }

                if (this.min_value[ch_min] == undefined || this.min_value[ch_min] > data[i]){
                    this.min_value[ch_min] = data[i]
                }
            }
        }

        if (this.max[ch_max] == undefined){
            this.max[ch_max] = new Float32Array(data)
        }else{
            for(let i = 0; i < data.length; i++){
                if (this.max[ch_max][i] < data[i]){
                    this.max[ch_max][i] = data[i]
                }

                if (this.max_value[ch_max] == undefined || this.max_value[ch_max] < data[i]){
                    this.max_value[ch_max] = data[i]
                }
            }
        }
    }

    getSignals(data) {
        for(let i = 1; i <=4; i++){
            let ch_min = "ch" + i + "_view_min"
            let ch_max = "ch" + i + "_view_max"
            if (this.min[ch_min]){
                data[ch_min] = {size: this.min[ch_min].length, value:this.min[ch_min]}
            }
            if (this.max[ch_max]){
                data[ch_max] = {size: this.max[ch_max].length, value:this.max[ch_max]}
            }
        }
    }

    resetByChannel(channel) {
        let ch_min = channel + "_min"
        let ch_max = channel + "_max"
        if (this.mode_channel[channel])
            this.mode_channel[channel] = undefined;
        if (this.min[ch_min])
            this.min[ch_min] = undefined;
        if (this.max[ch_max])
            this.max[ch_max] = undefined;
        if (this.min_value[ch_min])
            this.min_value[ch_min] = undefined;
        if (this.max_value[ch_max])
            this.max_value[ch_max] = undefined;
    }

    resetAll() {
        this.mode_channel = {}
        this.min = {}
        this.max = {}
        this.min_value = {}
        this.max_value = {}
    }

    resetByName(name){
        let isMin = name.includes('_MIN')
        let ch = name.slice(0, 3).toLowerCase()
        let key = ch + (isMin ? '_view_min' : '_view_max')
        if (this.mode_channel[key])
            this.mode_channel[key] = undefined;
        if (this.min[key])
            this.min[key] = undefined;
        if (this.max[key])
            this.max[key] = undefined;
        if (this.min_value[key])
            this.min_value[key] = undefined;
        if (this.max_value[key])
            this.max_value[key] = undefined;
    }

    getMax() {
        let max = undefined
        for(let i = 1; i <=4; i++){
            let ch_max = "ch" + i + "_view_max"
            if (this.max_value[ch_max]){
                max = max == undefined || max < this.max_value[ch_max] ? this.max_value[ch_max] : max
            }
        }
        return max
    }

    getMin() {
        let min = undefined
        for(let i = 1; i <=4; i++){
            let ch_min = "ch" + i + "_view_min"
            if (this.min_value[ch_min]){
                min = min == undefined || min > this.min_value[ch_min] ? this.min_value[ch_min] : min
            }
        }
        return min
    }

    getMaxOfVisible() {
        let max = undefined
        for(let i = 1; i <=4; i++){
            if (!SPEC.isVisibleChannel(i)) continue
            let ch_max = "ch" + i + "_view_max"
            if (this.max_value[ch_max]){
                max = max == undefined || max < this.max_value[ch_max] ? this.max_value[ch_max] : max
            }
        }
        return max
    }

    getMinOfVisible() {
        let min = undefined
        for(let i = 1; i <=4; i++){
            if (!SPEC.isVisibleChannel(i)) continue
            let ch_min = "ch" + i + "_view_min"
            if (this.min_value[ch_min]){
                min = min == undefined || min > this.min_value[ch_min] ? this.min_value[ch_min] : min
            }
        }
        return min
    }
}


$(function() {
    SPEC.minMaxController = new MinMaxController()
})
