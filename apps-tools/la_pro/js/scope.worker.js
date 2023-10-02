voltage_steps = [0.2, 0.4, 0.8, 1.6];
var repackSignals = function(signals) {
    var vals = {};
    var res = {};
    var hasData = false;
    for (var i = 1; i < 9; i++) {
        res["ch" + i] = {};
        res["ch" + i]["value"] = [];
        res["ch" + i]["size"] = 0;
    }

    for (var i = 0; i < signals["ch1"].size; i += 2) {
        var length = signals["ch1"].value[i] + 1;
        for (var chn = 0; chn < 8; chn++) {
            var ch = "ch" + (chn + 1);
            var val = (signals["ch1"].value[i + 1] >> chn) & 1;
            if (res[ch].value.length > 0) {
                if (val == (res[ch].value[res[ch].value.length - 1]))
                    res[ch].value[res[ch].value.length - 2] += length;
                else {
                    res[ch].value.push(length);
                    res[ch].value.push(val);
                }
            } else {
                res[ch].value.push(length);
                res[ch].value.push(val);
            }
        }
    }

    for (var k in res) {
        res[k]['size'] = res[k].value.length;
    }
    return res;
}

var repackSignalsAVG = function(signals, base) {
    BLOCK_SIZE = (base == undefined) ? 32 : base;
    BLOCK_SIZE = (BLOCK_SIZE >= 32) ? BLOCK_SIZE * 2 : BLOCK_SIZE;

    var vals = {};
    var res = {};
    var hasData = false;
    for (var i = 1; i < 9; i++) {
        var ch = "ch" + i;
        var ch_avg = ch; // + "_avg";

        res[ch_avg] = {};
        res[ch_avg]["value"] = [];
        res[ch_avg]["size"] = 0;

        var srcValues = signals[ch].value;
        var lastValue = undefined;
        var sum = 0;
        var counted = 0;
        var overflow = false;

        for (var j = 0; j < srcValues.length; j += 2) {
            if (srcValues[j] + counted >= BLOCK_SIZE) {
                var diff = BLOCK_SIZE - counted;
                counted += diff;
                if (srcValues[j + 1] == 1)
                    sum += diff;
                var coeff = sum / BLOCK_SIZE;
                var averagedValue = 0;
                if (coeff > 0.5)
                    averagedValue = 1;

                res[ch_avg]["value"].push(BLOCK_SIZE);
                res[ch_avg]["value"].push(averagedValue);

                var toCount = srcValues[j] - diff;
                while (toCount > BLOCK_SIZE) {
                    res[ch_avg]["value"].push(BLOCK_SIZE);
                    if (srcValues[j + 1] == 1)
                        res[ch_avg]["value"].push(1);
                    else
                        res[ch_avg]["value"].push(0);
                    toCount -= BLOCK_SIZE;
                }
                counted = toCount;
                sum = (srcValues[j + 1] == 1) ? toCount : 0;
            } else {
                counted += srcValues[j];
                if (srcValues[j + 1] == 1)
                    sum += srcValues[j];
            }
        }
        if (counted > 0) {
            var coeff = sum / BLOCK_SIZE;
            var averagedValue = 0;
            if (coeff > 0.5)
                averagedValue = 1;

            res[ch_avg]["value"].push(counted);
            res[ch_avg]["value"].push(averagedValue);
        }
    }
    for (var k in res) {
        res[k]['size'] = res[k].value.length;
    }
    return res;
}

var splitSignals = function(signals, OSC) {
        var vals = {};
        for (var i = 1; i < 9; i++)
            vals["ch" + i] = [];

        var offset = OSC.counts_offset;
        var offsetForDecoded = OSC.counts_offset;

        for (var chn = 0; chn < 8; chn++) {
            if (!OSC.enabled_channels[chn])
                continue;


            var overflow = false;
            var encoded = 0;
            var skip = 0;
            var first_skip = true;

            var ch_get = "ch" + (chn + 1); // + "_avg";
            var ch_set = "ch" + (chn + 1);

            var amount = 0;

            for (var i = 0; i < signals[ch_get].value.length; i += 2) {
                var length = signals[ch_get].value[i] * OSC.time_scale;

                if ((skip + length) >= offset) {
                    if (first_skip) {
                        length = (skip + length) - offset;
                        skip = offset;
                        first_skip = false;
                        encoded = amount = length;

                    } else {
                        if ((encoded + length) >= 1024) {
                            overflow = true;
                            amount = 1024 - encoded;
                        } else
                            amount = length;
                        encoded += amount;
                    }
                } else {
                    skip += length;
                    continue;
                }
                var amp = voltage_steps[OSC.voltage_index];
                var offset1 = OSC.voltage_offset[chn];
                var val = signals[ch_get].value[i + 1] * amp + offset1;
                vals[ch_set].push(amount);
                vals[ch_set].push(val);
            }

        }
        signals = {};
        for (var channel in vals) {
            signals[channel] = {};
            signals[channel].size = vals[channel].length;
            signals[channel].value = vals[channel];
        }
        var need_split = false;
        return signals;
    }
    // Processes newly received values for parameters

onmessage = function(e){
    var OSC = JSON.parse(e.data);
    if (OSC == null || OSC == undefined)
        return;
    var splitted_signal = {};
    var p = performance.now();
    if (OSC.need_split) {
        if (OSC.scaleWasChanged && OSC.time_scale < 0.25) {
            var base = 1 / OSC.time_scale;
            OSC.splittedAvgSignal = repackSignalsAVG(OSC.recv_signals, base);
            splitted_signal = splitSignals(OSC.splittedAvgSignal, OSC);
        } else
        {
            splitted_signal = (OSC.time_scale < 0.25) ? splitSignals(OSC.splittedAvgSignal, OSC) : splitSignals(OSC.recv_signals, OSC);
        }
    }
    console.log("Performance: " + (performance.now() - p));
    postMessage( JSON.stringify(splitted_signal));
}

