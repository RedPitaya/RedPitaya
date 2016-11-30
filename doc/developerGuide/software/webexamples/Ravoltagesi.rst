.. _ReadAVSI:

#######################################
Reading analog voltage from slow inputs
#######################################

| In this example we will print voltage measured on one of Red Pitaya slow analog inputs that are located on extension 
  connector :ref:`E2 <E2>`. 
| Notice that any of four AI pins (0-3) can be used.

******
Web UI
******

First of all you need new .js file:

**pako.js** - for decompress data

In **index.html** add:

.. code-block:: html

    <script src="js/jquery-2.1.3.min.js"></script>
    <script src="js/pako.js"></script>
    <script src="js/app.js"></script>
    
Our mesurement result will be in this block:

.. code-block:: html

    < div id='value'></div>

Add button to read voltage using this string in **index.html**:

.. code-block:: html

    <button id='read_button'>Read</button>

In **app.js** we should change **APP.ws.onmessage()** callback. We decompress message and process signals from it.

.. code-block:: html

    var data = new Uint8Array(ev.data);
    var inflate = pako.inflate(data);
    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
    var receive = JSON.parse(text);

    if (receive.signals) {
        APP.processSignals(receive.signals);
    }
    
Processing of signals is located in **APP.processSignals()** function. In this function we get voltage value from 
signal and print it in Web UI:

.. code-block:: html

    var voltage;

    for (sig_name in new_signals) {

        if (new_signals[sig_name].size == 0) continue;

        voltage = new_signals[sig_name].value[new_signals[sig_name].size - 1];

        $('#value').text(parseFloat(voltage).toFixed(2) + "V");
    }

By **APP.readValue()** we send request of reading voltage to controller.

.. code-block:: html

    var local = {};
    local['READ_VALUE'] = { value: true };
    APP.ws.send(JSON.stringify({ parameters: local }));
    
**********
Controller
**********

We read values from pins using controller, so in main.cpp we should make changes. Firstly add signal in global
variables:

.. code-block:: c

    CFloatSignal VOLTAGE("VOLTAGE", SIGNAL_SIZE_DEFAULT, 0.0f);

**SIGNAL_SIZE_DEFAULT** is our constant. It means how many measurements our signal will send to server. Now it is 1, 
because each time we need to send to Web UI only one value.

**VOLTAGE** is a name of our signal. It should be the same, as in **app.js**, in which we draw it on screen.

**0.0f** is default value of each measurement.

Also we need reading voltage parameter. It will

.. code-block:: c

    CBooleanParameter READ_VALUE("READ_VALUE", CBaseParameter::RW, false, 0);


Its’ default value is false. We will update this parameter in **OnNewParams()** function:

.. code-block:: c

    READ_VALUE.Update();

If **READ_VALUE.Value()** is **true** we will read value from **AIpin0** and write it to signal:

.. code-block:: c

    if (READ_VALUE.Value() == true)
    {
        float val;

        //Read data from pin
        rp_AIpinGetValue(0, &val);

        //Write data to signal
        VOLTAGE[0] = val;

        //Reset READ value
        READ_VALUE.Set(false);
    }

**val** - is buffer variable, which will get value from **AIpin0**. After writing data value will be sent to server. 
We should set **READ_VALUE** parameter to **false**.
