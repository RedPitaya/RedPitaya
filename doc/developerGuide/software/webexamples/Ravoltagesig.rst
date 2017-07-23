Reading analog voltage from slow inputs + graph
###############################################

In this example we will plot on graph voltage measured on one of Red Pitaya slow analog inputs. We take 
Reading analog voltage from slow inputs :ref:`example <ReadAVSI>` as a basis.

Web UI
******

You also need new .js file:

**jquery.flot.js** - for drawing graphs

.. code-block:: html

    <script src="js/jquery-2.1.3.min.js"></script>
    <script src="js/jquery.flot.js"></script>
    <script src="js/pako.js"></script>
    <script src="js/app.js"></script>

Add graph placeholder using this string in **index.html**:

.. code-block:: html

    < div id='placeholder'></div>

In **app.js** we should draw signal value on graph. Change **APP.ws.onmessage()** callback. Now we should decompress
message and push it to stack. Data arrives quite faster than we can process it. That’s why we should firsly save it,
and then process.

.. code-block:: html

    var data = new Uint8Array(ev.data);
    var inflate = pako.inflate(data);
    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
    var receive = JSON.parse(text);

    if (receive.signals) {
        APP.signalStack.push(receive.signals);
    }

Processing of signals is also located in **APP.processSignals()** function, which is called every 15ms by 
**APP.signalHandler()**. In this function we draw points according to values and update graph:

.. code-block:: none

    var pointArr = [];
    var voltage;

    for (sig_name in new_signals) {

        if (new_signals[sig_name].size == 0) continue;

        var points = [];
        for (var i = 0; i < new_signals[sig_name].size; i++) {
            points.push([i, new_signals[sig_name].value[i]]);
        }

        pointArr.push(points);

        voltage = new_signals[sig_name].value[new_signals[sig_name].size - 1];
    }

    $('#value').text(parseFloat(voltage).toFixed(2) + "V");

    APP.plot.setData(pointArr);
    APP.plot.resize();
    APP.plot.setupGrid();
    APP.plot.draw();

Controller
**********

As in a previous tutorial we will read values from pins using controller. In **main.cpp** we should make changes.

As you remember we added signal in global variables:

.. code-block:: c

    CFloatSignal VOLTAGE("VOLTAGE", SIGNAL_SIZE_DEFAULT, 0.0f);

Now **SIGNAL_SIZE_DEFAULT** should be 1024. We will send 1024 points to Web UI.

In **rp_app_init()** we should set signal update interval:

.. code-block:: c

    CDataManager::GetInstance()->SetSignalInterval(SIGNAL_UPDATE_INTERVAL);

**SIGNAL_UPDATE_INTERVAL** is also our constant. It is 10ms. It means how often program will call function void 
**UpdateSignals(void)**. In this function we will read value from **AIpin0** and write it to signal:

.. code-block:: c

    rp_AIpinGetValue(0, &val);

**val** - is buffer variable, which will get value from **AIpin0**. We should write this value to data vector in last 
position. First measurement should be deleted from this vector.

.. code-block:: c

    g_data.erase(g_data.begin());
    g_data.push_back(val * GAIN.Value());
    
After all steps write data to signal and it will be sent to server.

.. code-block:: c

    for(int i = 0; i < SIGNAL_SIZE_DEFAULT; i++) 
    {
        VOLTAGE[i] = g_data[i];
    }
