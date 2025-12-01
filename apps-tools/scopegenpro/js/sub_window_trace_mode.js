/*
 * Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(SW_TM, $, undefined) {
    SW_TM.windowId = "subWindow"
    SW_TM.isDragging = false
    SW_TM.subWindow = {}
    SW_TM.subWindow.offsetX = 0;
    SW_TM.subWindow.offsetY = 0;
    SW_TM.subWindowHandler = undefined
    SW_TM.isResizing = false;
    SW_TM.resizeHandle
    SW_TM.initialWidth
    SW_TM.initialHeight
    SW_TM.initialX
    SW_TM.initialY
    SW_TM.activeChannel = undefined

    SW_TM.openSubWindow = function() {
        if (SW_TM.subWindowHandler === undefined)
            return

        if (SW_TM.subWindowHandler.classList.contains('show')){
            return
        }
        SW_TM.subWindowHandler.classList.add('show');
        SW_TM.subWindowHandler.classList.remove('hidden');


        OSC.params.local['OSC_SW_TM_WIN_SHOW'] = {value : true}
        OSC.sendParams();
        SW_TM.checkSubWindowPosition()
    }

    SW_TM.closeSubWindow = function() {
        if (SW_TM.subWindowHandler === undefined)
            return

        if (SW_TM.subWindowHandler.classList.contains('hidden')){
            return
        }
        SW_TM.subWindowHandler.classList.add('hidden');
        SW_TM.subWindowHandler.classList.remove('show');
        OSC.params.local['OSC_SW_TM_WIN_SHOW'] = {value : false}
        OSC.sendParams();
    }

    SW_TM.initSubWindow = function(){
        SW_TM.subWindowHandler = document.getElementById(SW_TM.windowId);
        const resizeHandles = SW_TM.subWindowHandler.querySelectorAll('.resize-handle');
        const header = SW_TM.subWindowHandler.querySelectorAll('.subwindow-header');


        header.forEach(handle => {
            handle.addEventListener('mousedown', (e) => {
                SW_TM.isDragging = true;
                SW_TM.subWindowHandler.offsetX = e.clientX - SW_TM.subWindowHandler.offsetLeft;
                SW_TM.subWindowHandler.offsetY = e.clientY - SW_TM.subWindowHandler.offsetTop;
            });
        });


        resizeHandles.forEach(handle => {
            handle.addEventListener('mousedown', (e) => {
                SW_TM.isResizing = true;
                SW_TM.resizeHandle = handle;
                SW_TM.initialWidth = SW_TM.subWindowHandler.offsetWidth;
                SW_TM.initialHeight = SW_TM.subWindowHandler.offsetHeight;
                SW_TM.initialX = e.clientX;
                SW_TM.initialY = e.clientY;

                e.preventDefault(); //Prevent selection
                e.stopPropagation() // Stop drag
            });
        });


        document.addEventListener('mousemove', (e) => {
            if (SW_TM.isDragging){
                const x = e.clientX -  SW_TM.subWindowHandler.offsetX;
                const y = e.clientY - SW_TM.subWindowHandler.offsetY;
                SW_TM.subWindowHandler.style.left = x + 'px';
                SW_TM.subWindowHandler.style.top = y + 'px';
                OSC.params.local['OSC_SW_TM_WIN_X'] = {value: x}
                OSC.params.local['OSC_SW_TM_WIN_Y'] = {value: y}
                OSC.sendParams();
            }

            if (SW_TM.isResizing) {
                const deltaX = e.clientX - SW_TM.initialX;
                const deltaY = e.clientY - SW_TM.initialY;

                let newWidth = SW_TM.initialWidth;
                let newHeight = SW_TM.initialHeight;

                newWidth = SW_TM.initialWidth + deltaX;
                newHeight = SW_TM.initialHeight + deltaY;

                SW_TM.subWindowHandler.style.width = newWidth + 'px';
                SW_TM.subWindowHandler.style.height = newHeight + 'px';
                OSC.params.local['OSC_SW_TM_WIN_W'] = {value: newWidth}
                OSC.params.local['OSC_SW_TM_WIN_H'] = {value: newHeight}
                OSC.sendParams();
            }
        });

        document.addEventListener('mouseup', () => {
            SW_TM.isDragging = false;
            SW_TM.isResizing = false;
        });

        $('#SW_TM_RESET').on('click', function() {
            if (SW_TM.activeChannel){
                if (OSC.taMode['CH' + SW_TM.activeChannel])
                    OSC.taMode['CH' + SW_TM.activeChannel].clearBuffers()
            }
        });

        $('#SW_TM_TRACE_ENABLE').on('click', function() {
            if (SW_TM.activeChannel){
                var chkBox = document.getElementById('SW_TM_TRACE_ENABLE')
                var state = chkBox.getAttribute('data-checked') === "true"
                OSC.params.local['CH' + SW_TM.activeChannel + '_SHOW_TRACE'] = {value : !state}
                OSC.sendParams();
            }
        });

         $('#SW_TM_INVERTED').on('click', function() {
            if (SW_TM.activeChannel){
                var chkBox = document.getElementById('SW_TM_INVERTED')
                var state = chkBox.getAttribute('data-checked') === "true"
                OSC.params.local['CH' + SW_TM.activeChannel + '_TRACE_INVERTED'] = {value : !state}
                OSC.sendParams();
            }
        });

        $('#SW_TM_FAST_MODE').on('click', function() {
            var chkBox = document.getElementById('SW_TM_FAST_MODE')
            var state = chkBox.getAttribute('data-checked') === "true"
            OSC.params.local['OSC_SW_TM_FAST_MODE'] = {value : !state}
            OSC.sendParams();
        });



        $('.selected-color').on("input", (e) => {
            e.target.parentElement.style.backgroundColor = e.target.value;
        });

        $('.selected-color').on('change', (e) => {

            var colnum = undefined
            for(var i = 1; i <= 4; i++){
                if (e.target.id.includes("COLOR"+i)){
                    colnum = i
                }
            }

            var col_param = "CH" + SW_TM.activeChannel +  "_TRACE_COLOR" + colnum
            if (col_param){
                const s = OSC.params.orig[col_param]
                if (s){
                    var prevColor = s.value
                    const newColor = e.target.value;
                    if (newColor !== prevColor) {
                        OSC.params.local[col_param] = {value : newColor}
                        OSC.sendParams();
                    }
                }
            }
        });
    }



    SW_TM.setWinShow = function(new_params){
        if (new_params['OSC_SW_TM_WIN_SHOW'].value){
            SW_TM.openSubWindow()
            SW_TM.initTraceColor()
        }else{
            SW_TM.closeSubWindow()
        }

        var param_name = "OSC_SW_TM_WIN_SHOW";
        var state = new_params[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }
    }

    SW_TM.setWinX = function(new_params){
        if (SW_TM.isDragging == false && SW_TM.subWindowHandler !== undefined)
            SW_TM.subWindowHandler.style.left = new_params['OSC_SW_TM_WIN_X'].value + 'px';
    }

    SW_TM.setWinY = function(new_params){
        if (SW_TM.isDragging == false && SW_TM.subWindowHandler !== undefined)
            SW_TM.subWindowHandler.style.top = new_params['OSC_SW_TM_WIN_Y'].value + 'px';
    }

    SW_TM.setWinW = function(new_params){
        if (SW_TM.isResizing == false && SW_TM.subWindowHandler !== undefined)
            SW_TM.subWindowHandler.style.width = new_params['OSC_SW_TM_WIN_W'].value + 'px';
        // LOGGER.loadDecoderValues()
    }

    SW_TM.setWinH = function(new_params){
        if (SW_TM.isResizing == false && SW_TM.subWindowHandler !== undefined)
            SW_TM.subWindowHandler.style.height = new_params['OSC_SW_TM_WIN_H'].value + 'px';
    }

    SW_TM.setActiveChannel = function(new_params){
        SW_TM.activeChannel = new_params['OSC_SW_TM_CH_ACTIVE'].value
        $("#SW_TM_TITLE").text("Trace Mode - Channel #" + SW_TM.activeChannel)
        const s = OSC.params.orig['CH' + SW_TM.activeChannel + '_SHOW_TRACE']
        if (s)
            SW_TM.setTraceEnable(SW_TM.activeChannel,s.value)
        SW_TM.setTraceInverted(OSC.params.orig,'CH' + SW_TM.activeChannel + '_TRACE_INVERTED')
        SW_TM.initTraceColor()
    }

    SW_TM.setTraceEnable = function(channel,value) {
        if (SW_TM.activeChannel == channel){
            var chkBox = document.getElementById('SW_TM_TRACE_ENABLE');
            if (chkBox !== null){
                chkBox.setAttribute('data-checked', value == 1);
            }
        }
    }

    SW_TM.setFastMode = function(param,name){
         var chkBox = document.getElementById('SW_TM_FAST_MODE');
        if (chkBox !== null){
            chkBox.setAttribute('data-checked', param[name].value == 1);
        }
    }

    SW_TM.checkSubWindowPosition = function(){
        var isRectOutsideWindow = function(rect) {
            const windowWidth = window.innerWidth;
            const windowHeight = window.innerHeight;

            return (
              rect.right < 0 ||
              rect.left > windowWidth ||
              rect.bottom < 0 ||
              rect.top > windowHeight
            );
          }
        const subWindow = document.getElementById(SW_TM.windowId)
        if (subWindow !== null){
            const divRect = subWindow.getBoundingClientRect()
            const isOutside = isRectOutsideWindow(divRect)
            if (isOutside){
                OSC.params.local['OSC_SW_TM_WIN_X'] = {value: 300}
                OSC.params.local['OSC_SW_TM_WIN_Y'] = {value: 300}
                OSC.params.local['OSC_SW_TM_WIN_W'] = {value: 400}
                OSC.params.local['OSC_SW_TM_WIN_H'] = {value: 400}
                OSC.sendParams();
            }
        }
    }

    SW_TM.initTraceColor = function(){
        for(var i = 1; i <= 4; i++){
            if (SW_TM.activeChannel){
                var col_param = "CH" + SW_TM.activeChannel +  "_TRACE_COLOR" + i
                const s = OSC.params.orig[col_param]
                if (s){
                    $("#SW_TM_COLOR"+i).css({
                        "background-color": s.value
                    });

                    $("#SW_TM_COLOR"+i+"_INPUT").val(s.value);
                }
            }
        }
    }

    SW_TM.setTraceInverted = function(param,param_name){
        if (!(param_name in param)) return
        var ch = undefined
        for(var i = 1; i <= 4; i++){
            if (param_name.includes("CH"+i)){
                ch = i
            }
        }

        if (SW_TM.activeChannel && SW_TM.activeChannel == ch){
           var chkBox = document.getElementById('SW_TM_INVERTED');
            if (chkBox !== null){
                chkBox.setAttribute('data-checked', param[param_name].value == 1);
            }
        }
        if (OSC.taMode['CH' + SW_TM.activeChannel])
            OSC.taMode['CH' + SW_TM.activeChannel].setInverted(param[param_name].value)
    }

    SW_TM.setTraceColor = function(param,param_name){
        var ch = undefined
        var colnum = undefined
        for(var i = 1; i <= 4; i++){
            if (param_name.includes("CH"+i)){
                ch = i
            }
            if (param_name.includes("COLOR"+i)){
                colnum = i
            }
        }

        if (SW_TM.activeChannel && SW_TM.activeChannel == ch){
            $("#SW_TM_COLOR"+colnum).css({
                "background-color": param[param_name].value
            });

            $("#SW_TM_COLOR"+colnum+"_INPUT").val(param[param_name].value);
        }
        if (OSC.taMode['CH' + SW_TM.activeChannel])
            OSC.taMode['CH' + SW_TM.activeChannel].setColor(colnum,param[param_name].value)
    }


}(window.SW_TM = window.SW_TM || {}, jQuery));
