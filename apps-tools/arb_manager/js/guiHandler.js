/*
 * Red Pitaya arb manager
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

function promptFile(contentType, multiple) {
    var input = document.createElement("input");
    input.type = "file";
    input.accept = '.csv';
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

//Create callback
var changeCallbacks = {}

var clickCallbacks = {}

//Subscribe changes and clicks
$(document).ready(function() {
    for (var k in changeCallbacks) {
        $("#" + k).change(changeCallbacks[k]);
    }
    for (var i in clickCallbacks) {
        $("#" + i).click(clickCallbacks[i]);
    }

    $('#B_UPLOAD').click(function() {
        promptFile().then(function(file) {
            if(file){
                const fileReader = new FileReader(); // initialize the object
                fileReader.readAsArrayBuffer(file); // read file as array buffer
                fileReader.onload = (event) => {
                    console.log('Complete File read successfully!')
                    $.ajax({
                        url: '/upload_arb_file', //Server script to process data
                        type: 'POST',
                        //Ajax events
                        //beforeSend: beforeSendHandler,
                        success: function(e) {
                            console.log("Upload done " + e);
                            setTimeout(() => {
                                SM.parametersCache["RP_REQ_CHECK_FILE"] = { value: e };
                                SM.sendParameters();
                            }, 500);
                        },
                        error: function(e) { console.log(e); },
                        // Form data
                        data: event.target.result,
                        //Options to tell jQuery not to process data or worry about content-type.
                        cache: false,
                        contentType: false,
                        processData: false
                    });
                }
            }
            else
                console.log("no file selected")
        });
    });

})