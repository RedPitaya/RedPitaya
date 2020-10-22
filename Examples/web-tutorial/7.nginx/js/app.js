/*
 * Red Pitaya Template Application
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */


(function(APP, $, undefined) {
    
    // App configuration
    APP.config = {};
    APP.config.app_id = '7.nginx';
    APP.config.app_url = '/bazaar?start=' + APP.config.app_id + '?' + location.search.substr(1);
    APP.config.socket_url = 'ws://' + window.location.hostname + ':9002';

    // WebSocket
    APP.ws = null;




    // Starts template application on server
    APP.startApp = function() {

        $.get(APP.config.app_url)
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    APP.connectWebSocket();
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    APP.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    APP.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                APP.startApp();
            });
    };




    APP.connectWebSocket = function() {

        //Create WebSocket
        if (window.WebSocket) {
            APP.ws = new WebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            APP.ws = new MozWebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }


        // Define WebSocket event listeners
        if (APP.ws) {

            APP.ws.onopen = function() {
                $('#hello_message').text("Hello, Red Pitaya!");
                console.log('Socket opened');

                //Open root directory
                APP.openDir("/");
                
            };

            APP.ws.onclose = function() {
                console.log('Socket closed');
            };

            APP.ws.onerror = function(ev) {
                $('#hello_message').text("Connection error");
                console.log('Websocket error: ', ev);         
            };

            APP.ws.onmessage = function(ev) {
                console.log('Message recieved');
            };
        }
    };





    APP.openDir = function(dir) {

        $.get('/ngx_app_test?dir=' + dir + '').done(function(msg) {
                    var ngx_files = msg.split("\n"); 
                    APP.printFiles(ngx_files);
                });
    }



    APP.printFiles = function(files) {

        //Remove old files
        $('.child').remove();

        //Print new files
        for (var i = 0; i < files.length; i++){

            if (files[i] != ""){

                div = document.createElement('div');
                div.id = files[i] + "/";
                div.className = 'child';
                if (i == 0)
                    div.innerHTML = '<span class="child_desc">..</span>';
                else
                    div.innerHTML = '<span class="child_desc">' + files[i].split("/").pop() + '</span>';
                div.firstElementChild.onclick = function(){            
                    APP.openDir(this.parentNode.id);
                }
                file_system.appendChild(div);
            }
        }
    }


}(window.APP = window.APP || {}, jQuery));




// Page onload event handler
$(function() {

    // Start application
    APP.startApp();
});
