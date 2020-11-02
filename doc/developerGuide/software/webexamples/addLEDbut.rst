.. _ABCLED:

###########################
Add a button to control LED
###########################

You can control Red Pitaya's peripherals via Web UI. In this tutorial will be shown how to turn on and off LED on Red
Pitaya using parameters.

.. note::

	Requierment for manipulating leds using api is to first load fpga_0.94.bit fpga bitstream image.
	That can be done using next command line instruction:
	"cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg"


******
Web UI
******

Let’s start with UI, in index.html file we have to add a button that will be used to control LED::

    <button id='led_state'>Turn on</button>
    
and LED state label that will tell us if LED is On or Off. ::
    
    < div id='led_off'>LED Off</div>
    < div id='led_on'>LED On</div>
    
.. note:: 
    
    **led_on** div is not visible by default because when app starts all leds are off.

Also make some changes in **style.css** to set properties of these elements

.. code-block:: html

    #led_off {
        color: #F00;
    }

    #led_on {
        display: none;
        color: #0F0;
    }

    #led_state {
        margin-top: 20px;
        padding: 10px;
    }
    
Then we have to add some logic in app.js, that will be executed when user clicks on the button with the mouse. This 
logic should change local led_state each time button is clicked and send current led_state value to backend so that 
Red Pitaya can update real LED state.

.. code-block:: html

    APP.led_state = false;

    // program checks if led_state button was clicked
    $('#led_state').click(function() {

        // changes local led state
        if (APP.led_state == true){
            $('#led_on').hide();
            $('#led_off').show();
            APP.led_state = false;
        }
        else{
            $('#led_off').hide();
            $('#led_on').show();
            APP.led_state = true;
        }

        // sends current led state to backend
        var local = {};
        local['LED_STATE'] = { value: APP.led_state };
        APP.ws.send(JSON.stringify({ parameters: local }));
    });
    
 .. note::
    Parameter that transfers local LED state to Red Pitaya backend is called LED_STATE. You can change name of this 
    parameter, but don’t forget to use the same name also in controller.
    
**********
Controller
**********

After we send parameters we should read them in our controller. Controller source is located in ::

    src/main.cpp

This global variable is our parameter, that we should read from server.

.. code-block:: c
    
   CBooleanParameter ledState("LED_STATE", CBaseParameter::RW, false, 0); 
   
Parameter is a variable that connected with NGINX. Initialization has 4 arguments - parameter's name, access mode, 
initial value, and FPGA update flag. Pay attention - name of parameter LED_STATE should be the same as in app.js and 
type(bool - CBooleanParameter, int - CIntParameter, etc...) too.
This parameter updates in OnNewParams() function. This function is calling when new parameters arrived. In our case 
they will arrive each time you press the button in UI. 

.. code-block:: c

    ledState.Update();
    if (ledState.Value() == false)
    {
        rp_DpinSetState(RP_LED0, RP_LOW); 
    }
    else
    {
        rp_DpinSetState(RP_LED0, RP_HIGH); 
    }
    
    
**ledState.Update()** - updates value of parameter. It takes value from NGINX by parameter's name. That's why names
of parameters in **controller** and **app.js** should be the same.
**rp_DpinSetState** - is a Red Pitaya API function, which sets state of some pin. Its’ arguments are **rp_dpin_t** pin
and **rp_pinState_t *state**. In our program we control **RP_LED0**. There are 8 leds, thad we can control 
**RP_LED0 - RP_LED7**. 

There are two states of a LED - **RP_HIGH** (turned on) and **RP_LOW** (turned off).

Don’t forget to init **rpApp** and release it in **rp_app_init()** and **rp_app_exit()**.

Compile the controller, start app and try to push the button.   
