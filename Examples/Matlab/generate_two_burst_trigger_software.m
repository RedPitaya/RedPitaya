%% Define Red Pitaya as TCP/IP object
clc
clear all
close all
IP= ''; % Input IP of your Red Pitaya...
port = 5000; % If you are using WiFi then IP is:
tcpipObj=tcpip(IP, port);

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

%% The example generate sine bursts every 0.5 seconds indefinety

fprintf(tcpipObj,'GEN:RST');               % Reset to default settings

fprintf(tcpipObj,'SOUR1:FUNC SINE');
fprintf(tcpipObj,'SOUR1:FREQ:FIX 4'); % Set frequency of output signal
fprintf(tcpipObj,'SOUR1:VOLT 1'); % Set amplitude of output signal

fprintf(tcpipObj,'SOUR1:BURS:STAT BURST'); % Set burst mode to ON
fprintf(tcpipObj,'SOUR1:BURS:NCYC 2'); % Set 2 pulses of sine wave
fprintf(tcpipObj,'SOUR1:BURS:NOR 1'); % 1 number of sine wave pulses
fprintf(tcpipObj,'SOUR1:BURS:INT:PER 5000'); % Set time of burst period in microseconds = 5 * 1/Frequency * 1000000

fprintf(tcpipObj,'SOUR2:FUNC SINE');
fprintf(tcpipObj,'SOUR2:FREQ:FIX 4'); % Set frequency of output signal
fprintf(tcpipObj,'SOUR2:VOLT 1'); % Set amplitude of output signal

fprintf(tcpipObj,'SOUR2:BURS:STAT BURST'); % Set burst mode to ON
fprintf(tcpipObj,'SOUR2:BURS:NCYC 2'); % Set 2 pulses of sine wave
fprintf(tcpipObj,'SOUR2:BURS:NOR 1'); % 1 number of sine wave pulses
fprintf(tcpipObj,'SOUR2:BURS:INT:PER 5000'); % Set time of burst period in microseconds = 5 * 1/Frequency * 1000000
fprintf(tcpipObj,'OUTPUT:STATE ON'); % Set output to ON
pause(2)
fprintf(tcpipObj,'SOUR1:TRIG:INT');
pause(2)
fprintf(tcpipObj,'SOUR2:TRIG:INT');
pause(1)
fprintf(tcpipObj,'SOUR:TRIG:INT');

%% Close connection with Red Pitaya

fclose(tcpipObj);
