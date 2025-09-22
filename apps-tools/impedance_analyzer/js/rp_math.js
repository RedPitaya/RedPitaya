(function(RP_MATH, $, undefined) {

    RP_MATH.findMinMax = function(x, y) {
        if (x.length !== y.length || x.length === 0) {
            return undefined;
        }

        let minIndex = 0;
        let maxIndex = 0;

        for (let i = 1; i < y.length; i++) {
            if (y[i] < y[minIndex]) {
                minIndex = i;
            }
            if (y[i] > y[maxIndex]) {
                maxIndex = i;
            }
        }

        return {
            min: { x: x[minIndex], y: y[minIndex], index: minIndex },
            max: { x: x[maxIndex], y: y[maxIndex], index: maxIndex }
        };
    }

    RP_MATH.findMinMaxEx = function(x, y) {
        if (x.length !== y.length || x.length === 0) {
            return undefined;
        }

        let minIndex = 0;
        let maxIndex = 0;

        for (let i = 1; i < y.length; i++) {
            if (y[i] < y[minIndex]) {
                minIndex = i;
            }
            if (y[i] > y[maxIndex]) {
                maxIndex = i;
            }
        }

        if (minIndex == 0 || minIndex == y.length - 1) return undefined
        if (maxIndex == 0 || maxIndex == y.length - 1) return undefined

        return {
            min: { x: x[minIndex], y: y[minIndex], index: minIndex },
            max: { x: x[maxIndex], y: y[maxIndex], index: maxIndex }
        };
    }



    RP_MATH.calculateDataRange = function (x, y) {
        const xRange = Math.max(...x) - Math.min(...x);
        const yRange = Math.max(...y) - Math.min(...y);

        return {
            xRange: xRange,
            yRange: yRange,
            overallRange: Math.sqrt(xRange * xRange + yRange * yRange)
        };
    }

    RP_MATH.calculateNormalizedGradient = function (x, y, index, dataRange) {
        const firstDerivative = RP_MATH.calculateFirstDerivative(x, y);

        if (index < 0 || index >= firstDerivative.length) {
            return undefined
        }

        const rawGradient = firstDerivative[index];

        const normalizedGradient = rawGradient * (dataRange.xRange / dataRange.yRange);

        const maxPossibleGradient = Math.max(...firstDerivative.map(g => Math.abs(g * (dataRange.xRange / dataRange.yRange))));
        const absoluteNormalized = maxPossibleGradient > 0 ? Math.abs(normalizedGradient) / maxPossibleGradient : 0;

        return {
            raw: rawGradient,
            normalized: normalizedGradient,
            absoluteNormalized: absoluteNormalized
        };
    }

    RP_MATH.calculateFirstDerivative = function (x, y) {
        if (x.length !== y.length || x.length < 2) {
            return undefined
        }

        const derivative = [];

        derivative.push((y[1] - y[0]) / (x[1] - x[0]));

        for (let i = 1; i < x.length - 1; i++) {
            derivative.push((y[i + 1] - y[i - 1]) / (x[i + 1] - x[i - 1]));
        }

        derivative.push((y[y.length - 1] - y[y.length - 2]) / (x[x.length - 1] - x[x.length - 2]));

        return derivative;
    }

    RP_MATH.calculateGradientAtIndex = function (x, y, index) {
        const firstDerivative = RP_MATH.calculateFirstDerivative(x, y);

        if (index < 0 || index >= firstDerivative.length) {
            return undefined
        }

        return firstDerivative[index];
    }

     RP_MATH.calculateExtremeGradient = function (x, y, index) {
        const gradient =  RP_MATH.calculateGradientAtIndex(x, y, index);

        return {
            gradient: gradient,
            absoluteGradient: Math.abs(gradient)
        };
    }

    RP_MATH.findFirstOrderExtremes = function (x, y) {
        const firstDerivative = RP_MATH.calculateFirstDerivative(x, y);
        const extremes = [];
        const dataRange = RP_MATH.calculateDataRange(x, y);

        for (let i = 1; i < firstDerivative.length; i++) {
            if (firstDerivative[i - 1] * firstDerivative[i] <= 0) {
                let type;
                var extremeIndex = i
                if (firstDerivative[i - 1] > 0 && firstDerivative[i] < 0) {
                    type = 'maximum';
                    if (y[i - 1] > y[i]){
                        extremeIndex = i - 1
                    }
                } else if (firstDerivative[i - 1] < 0 && firstDerivative[i] > 0) {
                    type = 'minimum';
                    if (y[i - 1] < y[i]){
                        extremeIndex = i - 1
                    }
                } else {
                    continue;
                }

                const gradientInfo = RP_MATH.calculateNormalizedGradient(x, y, extremeIndex, dataRange);
                if (gradientInfo == undefined)
                    continue

                extremes.push({
                    x: x[extremeIndex],
                    y: y[extremeIndex],
                    index: extremeIndex,
                    type: type,
                    rawGradient: gradientInfo.raw,
                    normalizedGradient: gradientInfo.normalized,
                    maxPossibleGradient: gradientInfo.absoluteNormalized,
                    derivativeBefore: firstDerivative[i - 1],
                    derivativeAfter: firstDerivative[i]
                });
            }
        }

        return extremes;
    }

    RP_MATH.findAllExtremes = function(x, y) {
        return {
            globalMinMax: RP_MATH.findMinMaxEx(x, y),
            firstOrder:  RP_MATH.findFirstOrderExtremes(x, y)
        };
    }

}(window.RP_MATH = window.RP_MATH || {}, jQuery));

