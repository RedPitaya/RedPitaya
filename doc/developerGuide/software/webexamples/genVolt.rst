Generating voltage
##################

Take Reading analog voltage from slow inputs :ref:`example <ReadAVSI>` as a basic application for this example, because it is the 
simplest way to check generating voltage using one device. In this program we will set frequency, amplitude and 
waveform of generating signal.

Web UI
******

In **index.html** there are three new blocks - **frequency_setup, amplitude_setup** and **waveform_setup**.

.. code-block:: html

    < div id='frequency_setup'>
        < div>Frequency: Hz</div>
        <input id='frequency_set' type="range" size="2" value="1" min = "1" max = "20">
    </div>
    < div id='amplitude_setup'>
        < div>Amplitude: V</div>
        <input id='amplitude_set' type="range" step="0.01" size="2" value="0.5" min = "0" max = "0.5">
    </div>
    < div id='waveform_setup'>
        < div>Waveform</div>
        <select size="1" id="waveform_set">
            <option selected value="0">Sine</option>
            <option value="1">Sawtooth</option>
            <option value="2">Square</option>
        </select>
    </div>
    
In **app.js** we added three new functions: **APP.setFrequency(), APP.setAmplitude()** and **APP.setWaveform()**.

.. code-block:: html

    APP.setFrequency = function() {
        APP.frequency = $('#frequency_set').val();
        var local = {};
        local['FREQUENCY'] = { value: APP.frequency };
        APP.ws.send(JSON.stringify({ parameters: local }));
        $('#frequency_value').text(APP.frequency);
    };

    APP.setAmplitude = function() {
        APP.amplitude = $('#amplitude_set').val();
        var local = {};
        local['AMPLITUDE'] = { value: APP.amplitude };
        APP.ws.send(JSON.stringify({ parameters: local }));
        $('#amplitude_value').text(APP.amplitude);
    };

    APP.setWaveform = function() {
        APP.waveform = $('#waveform_set').val();
        console.log('Set to ' + APP.waveform);
        var local = {};
        local['WAVEFORM'] = { value: APP.waveform };
        APP.ws.send(JSON.stringify({ parameters: local }));
    };
    
    
Comtroller
**********

In **main.cpp** (controller) we added three 3 parameters:

.. code-block:: c

    CIntParameter FREQUENCY("FREQUENCY", CBaseParameter::RW, 1, 0, 1, 20);
    CFloatParameter AMPLITUDE("AMPLITUDE", CBaseParameter::RW, 0.5, 0, 0, 0.5);
    CIntParameter WAVEFORM("WAVEFORM", CBaseParameter::RW, 0, 0, 0, 2);
    
Minimum frequency is 1Hz and maximum - 20Hz. Minimum amplitude is 0 and maximum is 0.5, because our program can read 
voltage from slow inputs in range 0-3,3V and generator’s range is -1V +1V. We should set offset +0.5V and limit
amplitude’s maximum to 0.5V to get a signal in range 0V-1V(-0.5V + 0.5V is a range of generating signal and +0.5V
offset).

In our program waveform can be:

===== =============
value description
===== =============
    0  Sine
    1  Sawtooth
    2  Square
===== =============

There is a new function - **set_generator_config()**. In this function we configurate output signal. This api function 
sets frequency of our signal. Signal will be gererated on output channel 1(**RP_CH_1**).

.. code-block:: c

    rp_GenFreq(RP_CH_1, FREQUENCY.Value());

We need to set offset **0.5V**:

.. code-block:: c

    rp_GenOffset(RP_CH_1, 0.5);

Setting amplitude:

.. code-block:: c

    rp_GenAmp(RP_CH_1, AMPLITUDE.Value());
    
And setting waveform:

.. code-block:: c

    if (WAVEFORM.Value() == 0)
    {
        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
    }
    else if (WAVEFORM.Value() == 1)
    {
        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_RAMP_UP);
    }
    else if (WAVEFORM.Value() == 2)
    {
        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SQUARE);
    }
    
There can be other waveforms: **RP_WAVEFORM_TRIANGLE** (triangle), **RP_WAVEFORM_RAMP_DOWN** (reversed sawtooth), 
**RP_WAVEFORM_DC** (dc), **RP_WAVEFORM_PWM** (pwm), **RP_WAVEFORM_ARBITRARY** (defined wave form).

In **rp_app_init()** we should set up signal and turn it on:

.. code-block:: c

    set_generator_config();
    rp_GenOutEnable(RP_CH_1);
    
In **rp_app_exit()** disable signal:

.. code-block:: c

    rp_GenOutEnable(RP_CH_1);

And in OnNewParams() update parameters:

.. code-block:: c

    
    FREQUENCY.Update();
    AMPLITUDE.Update();
    WAVEFORM.Update();
