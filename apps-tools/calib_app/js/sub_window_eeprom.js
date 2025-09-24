/*
 * Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(SW, $, undefined) {
    SW.windowId = "subWindow"
    SW.isDragging = false
    SW.subWindow = {}
    SW.subWindow.offsetX = 0;
    SW.subWindow.offsetY = 0;
    SW.subWindowHandler = undefined
    SW.isResizing = false;
    SW.resizeHandle
    SW.initialWidth
    SW.initialHeight
    SW.initialX
    SW.initialY

    function download(url, filename) {
        fetch(url)
          .then(response => response.blob())
          .then(blob => {
            const link = document.createElement("a");
            link.href = URL.createObjectURL(blob);
            link.download = filename;
            link.click();
        })
        .catch(console.error);
    }

    SW.promptFile = function(contentType, multiple) {
        var input = document.createElement("input");
        input.type = "file";
        input.accept = '.bin';
        return new Promise(function(resolve) {
            document.activeElement.onfocus = function() {
            document.activeElement.onfocus = null;
                setTimeout(resolve, 500);
            };
            input.onchange = function() {
                var files = Array.from(input.files);
                resolve(files[0]);
            };
            input.click();
        });
    }

    SW.openSubWindow = function() {
        if (SW.subWindowHandler.classList.contains('show')){
            return
        }
        SW.subWindowHandler.classList.add('show');
        SW.subWindowHandler.classList.remove('hidden');
        CLIENT.parametersCache['SW_WIN_SHOW'] = {value : true}
        CLIENT.sendParameters()
        SW.checkSubWindowPosition()
    }

    SW.closeSubWindow = function() {
        if (SW.subWindowHandler.classList.contains('hidden')){
            return
        }
        SW.subWindowHandler.classList.add('hidden');
        SW.subWindowHandler.classList.remove('show');
        CLIENT.parametersCache['SW_WIN_SHOW'] = {value : false}
        CLIENT.sendParameters()
    }

    SW.initSubWindow = function(){
        SW.subWindowHandler = document.getElementById(SW.windowId);
        // const resizeHandles = SW.subWindowHandler.querySelectorAll('.resize-handle');
        const header = SW.subWindowHandler.querySelectorAll('.subwindow-header');


        header.forEach(handle => {
            handle.addEventListener('mousedown', (e) => {
                SW.isDragging = true;
                SW.subWindowHandler.offsetX = e.clientX - SW.subWindowHandler.offsetLeft;
                SW.subWindowHandler.offsetY = e.clientY - SW.subWindowHandler.offsetTop;
            });
        });


        // resizeHandles.forEach(handle => {
        //     handle.addEventListener('mousedown', (e) => {
        //         SW.isResizing = true;
        //         SW.resizeHandle = handle;
        //         SW.initialWidth = SW.subWindowHandler.offsetWidth;
        //         SW.initialHeight = SW.subWindowHandler.offsetHeight;
        //         SW.initialX = e.clientX;
        //         SW.initialY = e.clientY;

        //         e.preventDefault(); //Prevent selection
        //         e.stopPropagation() // Stop drag
        //     });
        // });


        document.addEventListener('mousemove', (e) => {
            if (SW.isDragging){
                const x = e.clientX -  SW.subWindowHandler.offsetX;
                const y = e.clientY - SW.subWindowHandler.offsetY;
                SW.subWindowHandler.style.left = x + 'px';
                SW.subWindowHandler.style.top = y + 'px';
                CLIENT.parametersCache['SW_WIN_X'] = {value: x}
                CLIENT.parametersCache['SW_WIN_Y'] = {value: y}
                CLIENT.sendParameters()
            }

            if (SW.isResizing) {
                const deltaX = e.clientX - SW.initialX;
                const deltaY = e.clientY - SW.initialY;

                let newWidth = SW.initialWidth;
                let newHeight = SW.initialHeight;

                newWidth = SW.initialWidth + deltaX;
                newHeight = SW.initialHeight + deltaY;

                SW.subWindowHandler.style.width = newWidth + 'px';
                SW.subWindowHandler.style.height = newHeight + 'px';
                CLIENT.parametersCache['SW_WIN_W'] = {value: newWidth}
                CLIENT.parametersCache['SW_WIN_H'] = {value: newHeight}
                CLIENT.sendParameters()
            }
        });

        document.addEventListener('mouseup', () => {
            SW.isDragging = false;
            SW.isResizing = false;
        });

        SW.calibVerStr = function(value){
            if (value == 1){
                return value + " / Ver. 125-14"
            }

            if (value == 2){
                return value + " / Ver. 250-12"
            }

            if (value == 3){
                return value + " / Ver. 125-14 4Ch"
            }

            if (value == 4){
                return value +  "/ Ver. 122-16"
            }

            if (value == 5){
                return value + " / Universal (API)"
            }

            if (value == 6){
                return value + " / Universal (FPGA)"
            }

            return value + " (No Info)"
        }

        SM.param_callbacks["SW_F_VER"] = function(param_name){
            if (param_name.value != -1){
                $("#SW_F_VER").text(SW.calibVerStr(param_name.value))
            }else{
                $("#SW_F_VER").text("ERROR")
            }
        };
        SM.param_callbacks["SW_F_COUNT"] = function(param_name){
            if (param_name.value != -1){
                $("#SW_F_COUNT").text(param_name.value)
            }else{
                $("#SW_F_COUNT").text("ERROR")
            }
        };
        SM.param_callbacks["SW_U_VER"] = function(param_name){
            if (param_name.value != -1){
                $("#SW_U_VER").text(SW.calibVerStr(param_name.value))
            }else{
                $("#SW_U_VER").text("ERROR")
            }
        };
        SM.param_callbacks["SW_U_COUNT"] = function(param_name){
            if (param_name.value != -1){
                $("#SW_U_COUNT").text(param_name.value)
            }else{
                $("#SW_U_COUNT").text("ERROR")
            }
        };

        $('#B_F_BACKUP_A').on('click', function(ev) {
            $.ajax({
                url: "/calib_app_create_backup?factory=1",
                type: 'GET',
                timeout: 5000
            }).done(function(res) {
                console.log(res)
                download("/calib_app/files/"+res.trim(),res.trim());
            }).fail(function(msg) {
                console.log('Error',msg);
                $('#info_dialog_label').text("Error download backup");
                $('#info_dialog').modal('show');
            });
        });

        $('#B_U_BACKUP_A').on('click', function(ev) {
            $.ajax({
                url: "/calib_app_create_backup?factory=0",
                type: 'GET',
                timeout: 5000
            }).done(function(res) {
                console.log(res)
                download("/calib_app/files/"+res.trim(),res.trim());
            }).fail(function(msg) {
                console.log('Error',msg);
                $('#info_dialog_label').text("Error download backup");
                $('#info_dialog').modal('show');
            });
        });

        $('#B_U_RESTORE_A').on('click', function(ev) {
            SW.promptFile().then(function(file) {
                if(file){
                    const fileReader = new FileReader(); // initialize the object
                    fileReader.readAsArrayBuffer(file); // read file as array buffer
                    const fsize = file.size
                    if (fsize > 1024 * 1024){
                        $('#info_dialog_label').text("The file is very large. The size is limited to 1MB.");
                        $('#info_dialog').modal('show');
                        return
                    }
                    fileReader.onload = (event) => {
                        console.log('Complete File read successfully!')
                        $.ajax({
                            url: '/calib_app_upload_backup', //Server script to process data
                            type: 'POST',
                            //Ajax events
                            //beforeSend: beforeSendHandler,
                            success: function(res) {
                                if (res.trim() == "OK"){
                                    console.log("Upload done",res);
                                    CLIENT.parametersCache["calib_sig"] = { value: 8 };
                                    CLIENT.sendParameters()
                                    OBJ.adcInitRequest();
                                }else{
                                    $('#info_dialog_label').text("Error upload backup");
                                    $('#info_dialog').modal('show');
                                    console.log(res);
                                }
                            },
                            error: function(e) {
                                $('#info_dialog_label').text("Error upload backup");
                                $('#info_dialog').modal('show');
                                console.log(e);
                            },
                            // Form data
                            data: event.target.result,
                            //Options to tell jQuery not to process data or worry about content-type.
                            cache: false,
                            contentType: false,
                            processData: false,
                            timeout: 60000,
                            xhr: function() {
                                var xhr = new XMLHttpRequest();
                                xhr.upload.addEventListener('progress', function(e) {
                                    if (e.lengthComputable) {
                                        var percent = Math.round((e.loaded / e.total) * 100);
                                        console.log(percent + '% uploaded');
                                    }
                                }, false);

                                return xhr;
                            }
                        });
                    }
                }
                else
                    console.log("No file selected")
            });
        });

        CLIENT.parametersCache["calib_sig"] = { value: 8 };
        CLIENT.sendParameters()
    }



    SW.setWinShow = function(state){
        if (state){
            SW.openSubWindow()
            CLIENT.parametersCache["calib_sig"] = { value: 8 };
            CLIENT.sendParameters()
        }else{
            SW.closeSubWindow()
        }

        var param_name = "SW_WIN_SHOW";
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addCSWss' : 'removeCSWss']('active');
        }
    }

    SW.setWinX = function(new_params){
        if (SW.isDragging == false)
            SW.subWindowHandler.style.left = new_params.value + 'px';
    }

    SW.setWinY = function(new_params){
        if (SW.isDragging == false)
            SW.subWindowHandler.style.top = new_params.value + 'px';
    }

    SW.setWinW = function(new_params){
        if (SW.isResizing == false){
            SW.subWindowHandler.style.width = new_params.value + 'px';
        }
    }

    SW.setWinH = function(new_params){
        if (SW.isResizing == false){
            SW.subWindowHandler.style.height = new_params.value + 'px';
        }
    }

    SW.checkSubWindowPosition = function(){
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
        const subWindow = document.getElementById(SW.windowId)
        const divRect = subWindow.getBoundingClientRect()
        const isOutside = isRectOutsideWindow(divRect)
        if (isOutside){
            CLIENT.parametersCache['SW_WIN_X'] = {value: 300}
            CLIENT.parametersCache['SW_WIN_Y'] = {value: 300}
            CLIENT.parametersCache['SW_WIN_W'] = {value: 400}
            CLIENT.parametersCache['SW_WIN_H'] = {value: 400}
            CLIENT.sendParameters()
        }
    }


}(window.SW = window.SW || {}, jQuery));
