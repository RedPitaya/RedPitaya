(function(RP_PLOT_INFO, $, undefined) {

    RP_PLOT_INFO.blocks = []

    RP_PLOT_INFO.fontSize = 18
    RP_PLOT_INFO.ext_offset  = 25
    RP_PLOT_INFO.ext_point_color = '#5eff00e0'
    RP_PLOT_INFO.ext_line_color = '#5eff0077'
    RP_PLOT_INFO.ext_text_background_color = '#0000007c'
    RP_PLOT_INFO.ext_text_color = '#fffffffb'

    RP_PLOT_INFO.min_max_min_inner_padding  = 20
    RP_PLOT_INFO.min_max_min_outer_padding  = 20
    RP_PLOT_INFO.min_max_min_line_extra_out  = 10
    RP_PLOT_INFO.min_max_line_color = '#56b7f8d5'
    RP_PLOT_INFO.min_max_text_color = '#fffffffb'

    RP_PLOT_INFO.fontName = "14px Helvetica Neue"

    RP_PLOT_INFO.roundValue =  function(num, decimals = 3){
        const rounded = Math.round(num * 1000) / 1000;
        const difference = Math.abs(num - rounded);
        if (difference < 1e-3) {
            return rounded;
        }
        return num;
    }

    RP_PLOT_INFO.isPointVisble = function(plot,x,y){
        var off = plot.getPlotOffset()
        if (x < off.left) return false
        if (y < off.top) return false

        if (x > off.left + plot.width()) return false
        if (y > off.top + plot.height()) return false
        return true
    }

    RP_PLOT_INFO.calculateBlockSize = function(text, ctx){
        ctx.font = RP_PLOT_INFO.fontName
        var s = ctx.measureText(text)
        return {w: s.width , h: (s.fontBoundingBoxAscent + s.fontBoundingBoxDescent)  }
    }

    RP_PLOT_INFO.hideIntersectingObjects = function (items) {
        if (!Array.isArray(items)) return items;
        const extIndices = [];
        for (let i = 0; i < items.length; i++) {
            const obj = items[i];
            if (obj && !obj.hidden && obj.type === "ext" && obj.values) {
                extIndices.push(i);
            }
        }
        for (let i = 0; i < extIndices.length; i++) {
            const currentIndex = extIndices[i];
            const currentObj = items[currentIndex];

            if (currentObj.hidden) continue;
            for (let j = i + 1; j < extIndices.length; j++) {
                const otherIndex = extIndices[j];
                const otherObj = items[otherIndex];
                if (otherObj.hidden) continue;
                if (RP_PLOT_INFO.doBoxesIntersect(currentObj.values, otherObj.values)) {
                    if (currentObj.values.normalizedGradient > otherObj.values.normalizedGradient) {
                        otherObj.hidden = true;
                    } else if (currentObj.values.normalizedGradient < otherObj.values.normalizedGradient) {
                        currentObj.hidden = true;
                        break;
                    }
                    else {
                        otherObj.hidden = true;
                    }
                }
            }
        }

        var min_max_obj = undefined
        for (let i = 0; i < items.length; i++) {
            var obj = items[i];
            if (obj && !obj.hidden && obj.type === "min_max" && obj.values) {
                min_max_obj = obj
                break
            }
        }
        if (min_max_obj !== undefined){
            for (let i = 0; i < items.length; i++) {
                var obj = items[i];
                if (obj && !obj.hidden && obj.type === "ext" && obj.values) {
                    if (RP_PLOT_INFO.doBoxesIntersect(min_max_obj.values, obj.values)) {
                       obj.hidden = true;
                    }
                }
            }
        }

        return items;
    }

    RP_PLOT_INFO.doBoxesIntersect = function (obj1, obj2) {
        const box1 = {
            left: obj1.boxCenter.left - obj1.boundBox.w / 2,
            right: obj1.boxCenter.left + obj1.boundBox.w / 2,
            top: obj1.boxCenter.top - obj1.boundBox.h / 2,
            bottom: obj1.boxCenter.top + obj1.boundBox.h / 2
        };

        const box2 = {
            left: obj2.boxCenter.left - obj2.boundBox.w / 2,
            right: obj2.boxCenter.left + obj2.boundBox.w / 2,
            top: obj2.boxCenter.top - obj2.boundBox.h / 2,
            bottom: obj2.boxCenter.top + obj2.boundBox.h / 2
        };

        const intersectX = box1.left <= box2.right && box1.right >= box2.left;
        const intersectY = box1.top <= box2.bottom && box1.bottom >= box2.top;
        return intersectX && intersectY;
    }

    RP_PLOT_INFO.preCalculateBlockDrawPosistion = function(plot, item){

        if (item.type == "ext"){
            var off = plot.getPlotOffset()
            var pointCenter = plot.pointOffset({
                x: item.values.x ,
                y: item.values.y
            });
            item.values.pointCenter = pointCenter
            item.values.boxCenter = Object.assign({}, pointCenter)

            var dir = item.values.type == "maximum" ? -1 : 1;
            item.values.boxCenter.top = item.values.boxCenter.top + RP_PLOT_INFO.ext_offset * dir

            var hidden = false

            if (item.values.boxCenter.top + item.values.boundBox.h / 2 + 10 >= off.top + plot.height()) {
                hidden = true
            }

            if (item.values.boxCenter.top - item.values.boundBox.h / 2 - 10 <= off.top) {
                hidden = true
            }

            if (item.values.boxCenter.left + item.values.boundBox.w / 2 >= off.left + plot.width() - 10) {
                item.values.boxCenter.left = off.left + plot.width() - 10 - item.values.boundBox.w / 2
            }

            if (item.values.boxCenter.left - item.values.boundBox.w / 2 <= off.left + 10) {
                item.values.boxCenter.left = off.left + 10 + item.values.boundBox.w / 2
            }

            item.hidden = hidden
        }

        if (item.type == "min_max"){
            var off = plot.getPlotOffset()
            var pointCenterMin = plot.pointOffset({
                x: item.values.min.x ,
                y: item.values.min.y
            });

            var pointCenterMax = plot.pointOffset({
                x: item.values.max.x ,
                y: item.values.max.y
            });
            item.values.pointCenterMin = pointCenterMin
            item.values.pointCenterMax = pointCenterMax
            var min_x = Math.min(pointCenterMax.left , pointCenterMin.left)
            var max_x = Math.max(pointCenterMax.left , pointCenterMin.left)
            var min_y = Math.min(pointCenterMax.top , pointCenterMin.top)
            var max_y = Math.max(pointCenterMax.top , pointCenterMin.top)
            if (item.values.pointCenterMin.left == min_x){
                item.values.isMinLeft = true
            }else{
                item.values.isMinLeft = false
            }
            item.hidden = false
            item.values.position = ""
            if ( (max_x - min_x) >= item.values.boundBox.w + RP_PLOT_INFO.min_max_min_inner_padding){
                // Draw between min and max
                var pointTextCenter = {left: (max_x - min_x) / 2.0 + min_x , top: (max_y - min_y ) / 2.0 + min_y}
                item.values.boxCenter = pointTextCenter
                item.values.position = "center"
            }else{
                var drawRight = true
                if (max_x + RP_PLOT_INFO.min_max_min_outer_padding + item.values.boundBox.w  > off.left + plot.width() - 10){
                    drawRight = false
                }

                if (min_x - RP_PLOT_INFO.min_max_min_outer_padding - item.values.boundBox.w < off.left + 10){
                    drawRight = undefined // Not enough space for rendering
                }

                if (drawRight !== undefined) {
                    if (drawRight) {
                        var pointTextCenter = {left: max_x + RP_PLOT_INFO.min_max_min_outer_padding + item.values.boundBox.w / 2.0 , top: (max_y - min_y ) / 2.0 + min_y}
                        item.values.boxCenter = pointTextCenter
                        item.values.position = "right"
                    }else{
                        var pointTextCenter = {left: min_x - RP_PLOT_INFO.min_max_min_outer_padding - item.values.boundBox.w / 2.0 , top: (max_y - min_y ) / 2.0 + min_y}
                        item.values.boxCenter = pointTextCenter
                        item.values.position = "left"
                    }
                }else{
                    item.hidden = true
                }
            }
        }
    }


    RP_PLOT_INFO.hideSubsequentSameYStrWithPriority = function (items) {
        if (!Array.isArray(items)) return items;

        const visibleObjects = [];
        for (let i = 0; i < items.length; i++) {
            const obj = items[i];
            if (obj && !obj.hidden && obj.type === "ext" && obj.values && obj.values.yStr) {
                visibleObjects.push({
                    index: i,
                    obj: obj,
                    left: obj.values.boxCenter?.left || 0
                });
            }
        }

        visibleObjects.sort((a, b) => a.left - b.left);

        const sequences = [];
        let currentSequence = [];
        let lastYStr = null;

        for (const item of visibleObjects) {
            const currentYStr = item.obj.values.yStr;

            if (currentYStr === lastYStr) {
                currentSequence.push(item);
            } else {
                if (currentSequence.length > 0) {
                    sequences.push(currentSequence);
                }
                currentSequence = [item];
                lastYStr = currentYStr;
            }
        }

        if (currentSequence.length > 0) {
            sequences.push(currentSequence);
        }

        for (const sequence of sequences) {
            if (sequence.length > 1) {
                let maxIndex = 0;
                let maxGradient = sequence[0].obj.values.normalizedGradient;

                for (let i = 1; i < sequence.length; i++) {
                    if (sequence[i].obj.values.normalizedGradient > maxGradient) {
                        maxGradient = sequence[i].obj.values.normalizedGradient;
                        maxIndex = i;
                    }
                }

                for (let i = 0; i < sequence.length; i++) {
                    if (i !== maxIndex) {
                        items[sequence[i].index].hidden = true;
                    }
                }
            }
        }

        return items;
    }

    RP_PLOT_INFO.setInfo = function(plot, ctx, info) {
        RP_PLOT_INFO.blocks = []
        if (info.globalMinMax !== undefined){
            var x = {type:"min_max",values: info.globalMinMax}
            var suff = info.scale != "" ? " " + info.scale : ""
            x.values.min.yStr = RP_PLOT_INFO.roundValue(x.values.min.y) + suff
            x.values.max.yStr = RP_PLOT_INFO.roundValue(x.values.max.y) + suff
            x.values.diffStr = RP_PLOT_INFO.roundValue(x.values.max.y - x.values.min.y)
            x.values.min.boundBox = RP_PLOT_INFO.calculateBlockSize( x.values.min.yStr ,ctx)
            x.values.max.boundBox = RP_PLOT_INFO.calculateBlockSize( x.values.max.yStr ,ctx)
            x.values.boundBox = RP_PLOT_INFO.calculateBlockSize( x.values.diffStr ,ctx)
            RP_PLOT_INFO.blocks.push(x)
        }

        if (info.firstOrder !== undefined){
            for (var it in info.firstOrder) {
                var x = {type:"ext",values: info.firstOrder[it]}
                var suff = info.scale != "" ? " " + info.scale : ""
                x.values.yStr = RP_PLOT_INFO.roundValue(x.values.y) + suff
                x.values.boundBox = RP_PLOT_INFO.calculateBlockSize(x.values.yStr,ctx)
                RP_PLOT_INFO.blocks.push(x)
            }
        }
    }

    RP_PLOT_INFO.draw = function(plot,ctx){
        for(var i = 0; i < RP_PLOT_INFO.blocks.length; i++){
            const itm = RP_PLOT_INFO.blocks[i]
            RP_PLOT_INFO.preCalculateBlockDrawPosistion(plot,itm)
        }
        RP_PLOT_INFO.hideIntersectingObjects(RP_PLOT_INFO.blocks)
        RP_PLOT_INFO.hideSubsequentSameYStrWithPriority(RP_PLOT_INFO.blocks)
        RP_PLOT_INFO.drawMinMax(plot,ctx)
        RP_PLOT_INFO.drawExt(plot,ctx)
    }

    RP_PLOT_INFO.drawMinMax = function(plot, ctx){
        for(var i = 0; i < RP_PLOT_INFO.blocks.length; i++){
            const itm = RP_PLOT_INFO.blocks[i]
            if (itm.type == "min_max" && itm.hidden == false){
                ctx.strokeStyle = RP_PLOT_INFO.min_max_line_color

                var min1Point = Object.assign({}, itm.values.pointCenterMin)
                var min2Point = Object.assign({}, itm.values.pointCenterMin)
                var max1Point = Object.assign({}, itm.values.pointCenterMax)
                var max2Point = Object.assign({}, itm.values.pointCenterMax)

                if (itm.values.position == "center"){
                    min1Point.left += (!itm.values.isMinLeft ? RP_PLOT_INFO.min_max_min_line_extra_out  : -RP_PLOT_INFO.min_max_min_line_extra_out)
                    min2Point.left = itm.values.boxCenter.left + (itm.values.isMinLeft ? RP_PLOT_INFO.min_max_min_line_extra_out  : -RP_PLOT_INFO.min_max_min_line_extra_out)

                    max1Point.left += (itm.values.isMinLeft ? RP_PLOT_INFO.min_max_min_line_extra_out  : -RP_PLOT_INFO.min_max_min_line_extra_out)
                    max2Point.left = itm.values.boxCenter.left + (!itm.values.isMinLeft ? RP_PLOT_INFO.min_max_min_line_extra_out  : -RP_PLOT_INFO.min_max_min_line_extra_out)

                }else if (itm.values.position == "right"){
                    min1Point.left +=  -RP_PLOT_INFO.min_max_min_line_extra_out
                    min2Point.left = itm.values.boxCenter.left + RP_PLOT_INFO.min_max_min_line_extra_out

                    max1Point.left +=  -RP_PLOT_INFO.min_max_min_line_extra_out
                    max2Point.left = itm.values.boxCenter.left + RP_PLOT_INFO.min_max_min_line_extra_out

                } if (itm.values.position == "left"){
                    min1Point.left +=  RP_PLOT_INFO.min_max_min_line_extra_out
                    min2Point.left = itm.values.boxCenter.left  -RP_PLOT_INFO.min_max_min_line_extra_out

                    max1Point.left += RP_PLOT_INFO.min_max_min_line_extra_out
                    max2Point.left = itm.values.boxCenter.left  -RP_PLOT_INFO.min_max_min_line_extra_out
                }
                if (!RP_PLOT_INFO.isPointVisble(plot,min1Point.left,min1Point.top) ||!RP_PLOT_INFO.isPointVisble(plot,min2Point.left,min2Point.top))
                    continue
                if (!RP_PLOT_INFO.isPointVisble(plot,max1Point.left,max1Point.top) ||!RP_PLOT_INFO.isPointVisble(plot,max2Point.left,max2Point.to))
                    continue

                ctx.beginPath();
                ctx.moveTo(min1Point.left, min1Point.top);
                ctx.lineTo(min2Point.left, min2Point.top);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(max1Point.left, max1Point.top);
                ctx.lineTo(max2Point.left, max2Point.top);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(itm.values.boxCenter.left, min1Point.top);
                ctx.lineTo(itm.values.boxCenter.left, max1Point.top);
                ctx.stroke();

                ctx.fillStyle = RP_PLOT_INFO.ext_text_background_color
                ctx.fillRect(itm.values.boxCenter.left - itm.values.boundBox.w / 2, itm.values.boxCenter.top - itm.values.boundBox.h / 2, itm.values.boundBox.w, itm.values.boundBox.h);
                ctx.fillStyle = RP_PLOT_INFO.min_max_text_color
                ctx.textAlign = "center";
                ctx.textBaseline = "middle"
                ctx.font = RP_PLOT_INFO.fontName
                ctx.fillText(itm.values.diffStr, itm.values.boxCenter.left , itm.values.boxCenter.top);
            }
        }
    }

    RP_PLOT_INFO.drawExt = function(plot, ctx){

        for(var i = 0; i < RP_PLOT_INFO.blocks.length; i++){
            const itm = RP_PLOT_INFO.blocks[i]
            if (itm.type == "ext" && itm.hidden == false){
                if (RP_PLOT_INFO.isPointVisble(plot,itm.values.pointCenter.left,itm.values.pointCenter.top)){
                    if (true){
                        ctx.strokeStyle = RP_PLOT_INFO.ext_line_color
                        ctx.beginPath();
                        ctx.moveTo(itm.values.pointCenter.left, itm.values.pointCenter.top);
                        ctx.lineTo(itm.values.boxCenter.left, itm.values.boxCenter.top);
                        ctx.stroke();
                    }

                    ctx.beginPath();
                    ctx.arc(itm.values.pointCenter.left, itm.values.pointCenter.top, 4, 0, Math.PI * 2);
                    ctx.fillStyle = RP_PLOT_INFO.ext_point_color
                    ctx.fill();

                    if (true){
                        ctx.fillStyle = RP_PLOT_INFO.ext_text_background_color
                        ctx.fillRect(itm.values.boxCenter.left - itm.values.boundBox.w / 2, itm.values.boxCenter.top - itm.values.boundBox.h / 2, itm.values.boundBox.w, itm.values.boundBox.h);
                        ctx.fillStyle = RP_PLOT_INFO.ext_text_color
                        ctx.textAlign = "center";
                        ctx.textBaseline = "middle"
                        ctx.font = RP_PLOT_INFO.fontName
                        ctx.fillText(itm.values.yStr, itm.values.boxCenter.left , itm.values.boxCenter.top);
                    }

                }
            }
        }
    }

}(window.RP_PLOT_INFO = window.RP_PLOT_INFO || {}, jQuery));

