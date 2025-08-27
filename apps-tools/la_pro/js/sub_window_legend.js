/*
 * Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(LA, $, undefined) {
    LA.windowId = "subWindow"
    LA.isDragging = false
    LA.subWindow = {}
    LA.subWindow.offsetX = 0;
    LA.subWindow.offsetY = 0;
    LA.subWindowHandler = undefined
    LA.isResizing = false;
    LA.resizeHandle
    LA.initialWidth
    LA.initialHeight
    LA.initialX
    LA.initialY

    LA.openSubWindow = function() {
        if (LA.subWindowHandler.classList.contains('show')){
            return
        }
        LA.subWindowHandler.classList.add('show');
        LA.subWindowHandler.classList.remove('hidden');
        CLIENT.parametersCache['LA_WIN_SHOW'] = {value : true}
        CLIENT.sendParameters()
        LA.checkSubWindowPosition()
        LOGGER.loadDecoderValues()
    }

    LA.closeSubWindow = function() {
        if (LA.subWindowHandler.classList.contains('hidden')){
            return
        }
        LA.subWindowHandler.classList.add('hidden');
        LA.subWindowHandler.classList.remove('show');
        CLIENT.parametersCache['LA_WIN_SHOW'] = {value : false}
        CLIENT.sendParameters()
    }

    LA.initSubWindow = function(){
        LA.subWindowHandler = document.getElementById(LA.windowId);
        const resizeHandles = LA.subWindowHandler.querySelectorAll('.resize-handle');
        const header = LA.subWindowHandler.querySelectorAll('.subwindow-header');


        header.forEach(handle => {
            handle.addEventListener('mousedown', (e) => {
                LA.isDragging = true;
                LA.subWindowHandler.offsetX = e.clientX - LA.subWindowHandler.offsetLeft;
                LA.subWindowHandler.offsetY = e.clientY - LA.subWindowHandler.offsetTop;
            });
        });


        resizeHandles.forEach(handle => {
            handle.addEventListener('mousedown', (e) => {
                LA.isResizing = true;
                LA.resizeHandle = handle;
                LA.initialWidth = LA.subWindowHandler.offsetWidth;
                LA.initialHeight = LA.subWindowHandler.offsetHeight;
                LA.initialX = e.clientX;
                LA.initialY = e.clientY;

                e.preventDefault(); //Prevent selection
                e.stopPropagation() // Stop drag
            });
        });


        document.addEventListener('mousemove', (e) => {
            if (LA.isDragging){
                const x = e.clientX -  LA.subWindowHandler.offsetX;
                const y = e.clientY - LA.subWindowHandler.offsetY;
                LA.subWindowHandler.style.left = x + 'px';
                LA.subWindowHandler.style.top = y + 'px';
                CLIENT.parametersCache['LA_WIN_X'] = {value: x}
                CLIENT.parametersCache['LA_WIN_Y'] = {value: y}
                CLIENT.sendParameters()
            }

            if (LA.isResizing) {
                const deltaX = e.clientX - LA.initialX;
                const deltaY = e.clientY - LA.initialY;

                let newWidth = LA.initialWidth;
                let newHeight = LA.initialHeight;

                newWidth = LA.initialWidth + deltaX;
                newHeight = LA.initialHeight + deltaY;

                LA.subWindowHandler.style.width = newWidth + 'px';
                LA.subWindowHandler.style.height = newHeight + 'px';
                CLIENT.parametersCache['LA_WIN_W'] = {value: newWidth}
                CLIENT.parametersCache['LA_WIN_H'] = {value: newHeight}
                CLIENT.sendParameters()
                LOGGER.buildDecodedHeader()
                LOGGER.clearDecoderValues()
                LOGGER.setDecodedSize()
            }
        });

        document.addEventListener('mouseup', () => {
            LA.isDragging = false;
            LA.isResizing = false;
            LOGGER.buildDecodedHeader()
            LOGGER.buildDecoderData()
            LOGGER.setDecodedSize()
        });

        $('#LA_DATA_FILTER').on('input', function(){ 
            LOGGER.buildDecoderData()
        })

    }



    LA.setWinShow = function(new_params){
        if (new_params['LA_WIN_SHOW'].value){
            LA.openSubWindow()
        }else{
            LA.closeSubWindow()
        }

        var param_name = "LA_WIN_SHOW";
        var state = new_params[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }
    }

    LA.setWinX = function(new_params){
        if (LA.isDragging == false)
            LA.subWindowHandler.style.left = new_params['LA_WIN_X'].value + 'px';
    }

    LA.setWinY = function(new_params){
        if (LA.isDragging == false)
            LA.subWindowHandler.style.top = new_params['LA_WIN_Y'].value + 'px';
    }

    LA.setWinW = function(new_params){
        if (LA.isResizing == false){
            LA.subWindowHandler.style.width = new_params['LA_WIN_W'].value + 'px';
            LOGGER.loadDecoderValues()
        }
    }

    LA.setWinH = function(new_params){
        if (LA.isResizing == false){
            LA.subWindowHandler.style.height = new_params['LA_WIN_H'].value + 'px';
        }
    }

    LA.checkSubWindowPosition = function(){
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
        const subWindow = document.getElementById(LA.windowId)
        const divRect = subWindow.getBoundingClientRect()
        const isOutside = isRectOutsideWindow(divRect)
        if (isOutside){
            CLIENT.parametersCache['LA_WIN_X'] = {value: 300}
            CLIENT.parametersCache['LA_WIN_Y'] = {value: 300}
            CLIENT.parametersCache['LA_WIN_W'] = {value: 400}
            CLIENT.parametersCache['LA_WIN_H'] = {value: 400}
            CLIENT.sendParameters()
        }
    }


}(window.LA = window.LA || {}, jQuery));
