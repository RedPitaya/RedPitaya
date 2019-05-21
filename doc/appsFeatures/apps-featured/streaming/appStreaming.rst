#######################
Streaming
#######################

Streaming application enables user to stream data from STEMlab to file stored on STEMlab SD
card or over ethernet using UDP or TCP protocol. User is able to set sampling frequency,
number of input channels and resolution.


**********************************************
Start using STEMlab streaming feature
**********************************************

#. ) Run streaming app from STEMlab WEB interface

    .. image:: img/redpitaya_main_page.png
    
    .. image:: img/streaming_main_page.png

#. ) Stream locally to a file

    * Select Local file option and press RUN.
    * Streaming can be stopped by pressing the STOP button.
    * To get streaming results click Browse and download streaming data file.

#. ) Stream and transfer data over network

    * Select TCP/IP option and press RUN.
    * Download streaming client to your computer.

    `Linux <http://downloads.redpitaya.com/downloads/STEMlab-122-16/ecosystems/streaming/linux-tool.zip>`__.
    `Windows <http://downloads.redpitaya.com/downloads/STEMlab-122-16/ecosystems/streaming/windows-tool.zip>`__.

    * Run streaming on STEMlab by pressing RUN button
    * Open console terminal & start streaming client cmd line tool. Make sure to set right STEMlab IP and transfer parameters.
    * Streaming can be stopped by hitting Ctrl-C.
    * Two files are created each time streaming is started, one holds streamed data and the other one data transfer report.
    
    .. image:: img/win_client.png