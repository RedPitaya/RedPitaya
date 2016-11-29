#################################################################
Reading analog voltage from slow inputs + graph + gain and offset
#################################################################

In this example we will modify our oscilloscope made in Reading analog voltage from slow inputs
:ref:`example <ReadAVSI>`. We will add gain and offset settings to present how some parameters set in UI can be then 
applied on the signal in the backend.

******
Web UI
******

In **index.html** we need to add gain and offset blocks.
Without gain some measurements may be very low and offset can set minimal voltage.

.. code-block:: html

        
    < div id='gain_setup'>
        < div>Gain: </div>
        <input id='gain_set' type="range" size="2" value="1" min = "1" max = "100">
    </div>
    
Offset: 

.. code-block:: html

    
    <input id='offset_set' type="range" size="2" value="0" min = "0" max = "5" step="0.1">

In **app.js** we should set gain and offset by **APP.setGain** and **APP.setOffset** and send them to server.

They will be used by controller.

.. code-block:: html

    APP.gain = $('#gain_set').val();

    var local = {};
    local['GAIN'] = { value: APP.gain };
    APP.ws.send(JSON.stringify({ parameters: local }));

    $('#gain_value').text(APP.gain);



    APP.offset = $('#offset_set').val();

    var local = {};
    local['OFFSET'] = { value: APP.offset };
    APP.ws.send(JSON.stringify({ parameters: local }));

    $('#offset_value').text(APP.offset);

**********
Controller
**********

In **main.cpp** we need new parameters.

**Gain:**

.. code-block:: c

    CIntParameter GAIN("GAIN", CBaseParameter::RW, 1, 0, 1, 100);

Its’ min value is 1 and max is 100. By default it is 1.

**Offset:**

.. code-block:: c

    
    CFloatParameter OFFSET("OFFSET", CBaseParameter::RW, 0.0, 0, 0.0, 5.0);

Its’ min value is **0.0** and max is **5.0**. By default it is **0.0**.

They will be updated in **OnNewParams()** function:

.. code-block:: c

    GAIN.Update();
    OFFSET.Update();

We should modify writing to signal in **UpdateSignals().**

Value needed to be multiplied by gain and add offset.

.. code-block:: c

    for(int i = 0; i < SIGNAL_SIZE_DEFAULT; i++) 
    {
        VOLTAGE[i] = g_data[i] * GAIN.Value() + OFFSET.Value();
    }
