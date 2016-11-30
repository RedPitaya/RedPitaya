Nginx requests
##############

You can execute system commands via Nginx requests. For this tutorial take Creating first app as basis. We will write 
filemanager using Nginx location.

Web UI
******

In index.html create a new block:


.. code-block:: html
   
   < div id="file_system"></div> 
   
It will show content of current folder.

In **app.js** there are two new functions - **APP.openDir()** and **APP.printFiles()**.

In **APP.openDir()**:

.. code-block:: html

    $.get('/ngx_app_test?dir=' + dir + ).done(function(msg) {
        var ngx_files = msg.split("\n"); 
        APP.printFiles(ngx_files);
    });

**$.get** method sends parameter dir to server and loads data. If loading was successful, **done** method is called. 
In **done** method we split received data to get list of files and folders. Then we print them calling 
**APP.printFiles()** function.

In **APP.printFiles()** :

.. code-block:: none

    $('.child').remove();
    
    for (var i = 0; i < files.length; i++){
        if (files[i] != ""){
            div = document.createElement('div');
            div.id = files[i] + "/";
            div.className = 'child';
            if (i == 0)
                div.innerHTML = '..';
            else
                div.innerHTML = '' + files[i].split("/").pop() + '';
            div.firstElementChild.onclick = function(){            
                APP.openDir(this.parentNode.id);
            }
            file_system.appendChild(div);
        }
    }
    
First of all we should clean screen from old data. **$('.child').remove()**; deletes all elements with class **child**
. Then we print new files with class **child** and set them **onclick** listeners. In **onclick** we open a new directory.

In **APP.ws.onopen()** callback we should open a root directory:

.. code-block:: html

    APP.openDir("/");

    
Nginx location
**************

There is a new project file - nginx.conf. Content of this file:   

.. code-block:: none

   location /ngx_app_test {
       add_header 'Access-Control-Allow-Origin' '*';
       add_header 'Access-Control-Allow-Credentials' 'true';
       add_header 'Access-Control-Allow-Methods' 'GET, POST, OPTIONS';
       add_header 'Access-Control-Allow-Headers' 'DNT,X-Mx-ReqToken,Keep-Alive,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type';
       add_header 'Content-type' 'text/plain; charset=utf-8'; 

       content_by_lua '
           local args = ngx.req.get_uri_args()
           if args.dir then
               os.execute("(dirname "..args.dir.." && ls -d "..args.dir.."*) > /tmp/ngx_file_system");
               local handle = io.open("/tmp/ngx_file_system", "r");
               local res = handle:read("*all");
               io.close(handle);
               ngx.say(res);
           end        
       ';
   }
     

In **content_by_lua** section there is main logic of request.

Server gets **args.dir** param, which was sent from **app.js**. If it is not empty server executes system command to 
get parent directory and list of files of current directory. Then it reads result from temporary file and sends it to 
client.

After all steps you will get an application with file manager.

**Reboot** your Red Pitaya to apply new NGINX location.


.. code-block:: shell-session 

    # reboot
    
and then start application.

Now you can open Red Pitaya's folders and see their contents by Web UI.
