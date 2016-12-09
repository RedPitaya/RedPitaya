Creating first app
##################

Before you start creating your first application you need to set your development environment. Instructions how to do
that are in article Setting development environment. Also it's recommended to read brief System overview in order to 
understand what are the main components of system and how they communicate with each other.

Preparations
************

First of all you need to connect to your Red Pitaya via SSH. Follow this instructions SSH connection or simply open 
SSH shells in Eclipse.
After successful connection execute rw command in order to make file-system writable:

.. code-block:: shell-session

   $ rw

Also you need to install Git for cloning Red Pitaya project from GitHub. It will help you to manage changes.

.. code-block:: shell-session

   # apt-get install git
   
After installing you should configure it:

.. code-block:: shell-session

   $ git config --global user.name "username"
   $ git config --global user.email "username@mail.com"
   
where ``username`` is your or any other name, and ``username@mail.com`` is your email.

When these steps are done go to root directory and clone Red Pitaya Project:

.. code-block:: shell-session

   $ cd /root/ 
   $ git clone https://github.com/RedPitaya/RedPitaya.git
   
Examples will be situated in "/root/RedPitaya/Examples/web-tutorial/" folder.
All preparations were done. Let's go!

Ecosystem structure
*******************

As you know from System overview application contains two parts. They are frontend and backend. Backend contains all
required files for working with hardware of Red Pitaya. You can find your applications in::

   /opt/redpitaya/www/apps/
   
This is done for ease of use all applications. All available FPGA images can be found here::

    /opt/redpitaya/fpga
    
All libraries you may need to link your app with can be found here::

    /opt/redpitaya/lib
    
Project structure
*****************

Each application folder contains both frontend and backend files in same location. Using specific directory structure 
you will not have a mess between UI files and your controller. Frontend is web-based application so it requires HTML 
code for layout, CSS for elements styles, and JavaScript for application logic. Let have look on it first.
At first you need to copy "1.template" folder to "/opt/redpitaya/www/apps" directory and rename it, for example 
"myFirstApp".
   
.. code-block:: shell-session

   $ cd /opt/redpitaya/www/apps
   $ cp -r /root/RedPitaya/Examples/web-tutorial/1.template ./myFirstApp
   $ cd myFirstApp
   
This will be your application folder. Notice: the name of the application folder defines unique Application ID!

You can edit application name & description in /info/info.json file. ::

    {
        "name": “My First App",
        "version": "0.91-BUILD_NUMBER",
        "revision": "REVISION",
        "description": "This is my first app."
    }

Application icon image is "/info/icon.png". You may also change it.

Modify application title in index.html file:  

.. code-block:: html

   <!DOCTYPE html>
    <html lang="en">

    <head>
        <meta http-equiv="content-type" content="text/html; charset=utf-8"></meta>
        <title>My Application</title>
        <link rel="stylesheet" href="css/style.css">
        <script src="js/jquery-2.1.3.min.js"></script>
        <script src="js/app.js"></script>
    </head>

    <body>
        < div id='hello_message'>
            Connecting...
        < /div>
    </body>
    </html>
    
 Obviously you may want to have your own unique look of application. For that case you need to edit file:: 
 
 css/style.css
 
By default it contains this code: 

.. code-block:: html

    html,
    body {
        width: 100%;
        height: 100%;
    }

    body {
        color: #cdcccc;
        overflow: auto;
        margin: 0;
    }

    #hello_message{
        width: 500px;
        height: 250px;
        margin: 0 auto;
        background-color: #333333;
        text-align: center;
    }

JavaScript application establishes connection with your Red Pitaya::

    js/app.js
    
You should change application id to name of your application folder. From::

    APP.config.app_id = '1.template';
    
to::

    APP.config.app_id = 'myFirstApp';
    
Entry point of JS is **APP.startApp().** It sends request for loading application status. If status is not "OK" request 
will be sent again. If application was loaded JS application tries to connect to Red Pitaya via WebSocket calling 
**APP.connectWebSocket().**

.. code-block:: html

   if (window.WebSocket) {
       APP.ws = new WebSocket(APP.config.socket_url);
       APP.ws.binaryType = "arraybuffer";
   } else if (window.MozWebSocket) {
       APP.ws = new MozWebSocket(APP.config.socket_url);
       APP.ws.binaryType = "arraybuffer";
   } else {
       console.log('Browser does not support WebSocket');
   }

   if (APP.ws) {

       APP.ws.onopen = function() {
           $('#hello_message').text("Hello, Red Pitaya!");
           console.log('Socket opened');               
       };

       APP.ws.onclose = function() {
           console.log('Socket closed');
       };

       APP.ws.onerror = function(ev) {
            $('#hello_message').text("Connection error");
            console.log('Websocket error: ', ev);         
       };

       APP.ws.onmessage = function(ev) {
            console.log('Message received');
       };
   }
   
First of all application checks if there is WebSocket support in browser. Then new WebSocket connection creates.
There are four WebSocket callbacks:

   - **APP.ws.onopen()** - called when socket connection was successfully opened
   - **APP.ws.onclose()** - called when socket connection was successfully closed
   - **APP.ws.onerror()** - called when there is an error in establishing socket connection
   - **APP.ws.onmessage()** - called when message was received
   
Backend is a C/C++ application which controls Red Pitaya peripherals. Source code of this application is stored in src folder. It can be compiled intro controller.

| Main file must contain 11 mandatory functions:
| **const char *rp_app_desc(void)** - returns application description
| **int rp_app_init(void)** - called when application was started
| **int rp_app_exit(void)** - called when application was closed
| **int rp_set_params(rp_app_params_t *p, int len) -** 
| **int rp_get_params(rp_app_params_t **p) -** 
| **int rp_get_signals(float ***s, int *sig_num, int *sig_len) -** 
| **void UpdateSignals(void)** - updates signals(you should set update interval)
| **void UpdateParams(void)** - updates parametes(you should set update interval)
| **void OnNewParams(void)** - called when parameters were changed
| **void OnNewSignals(void)** - called when signals were changed
| **void PostUpdateSignals(void)** - 

This functions are called by NGINX. We will add some code into this part later.

Also there is a file called **fpga.conf**. It defines which FPGA image is loaded when application is started (FPGA images are located in /opt/redpitaya/fpga).

Compiling application
*********************

To compile application run in /opt/redpitaya/www/apps/**<your_app_name>** folder on Red Pitaya:

.. code-block:: shell-session

   $ cd /opt/redpitaya/www/apps/myFirstApp/
   $ make INSTALL_DIR=/opt/redpitaya
   
Compiling process will start. After comping will be created file “controller.so”. Try to connect to Red Pitaya in 
browser. Application should appear in the list. Notice: compiling is needed if you haven't compile it yet or change 
source files. If you change only WEB files don't recompile.   
   
