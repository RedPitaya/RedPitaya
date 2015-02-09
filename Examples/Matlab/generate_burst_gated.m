%% Define Red Pitaya as TCP/IP object
clc
clear all
close all
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:              
tcpipObj=tcpip(IP, port);       % 192.168.128.1
tcpipObj.InputBufferSize = 16384*64;
tcpipObj.OutputBufferSize = 16384*64;
flushinput(tcpipObj)
flushoutput(tcpipObj)

%% Open connection with your Red Pitaya
x=instrfind;
fclose(x);
fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

%%

fprintf(tcpipObj,'GEN:RST')                     % Reset to default settings

fprintf(tcpipObj,'SOUR1:FUNC SINE');            % Set function of output signal     
fprintf(tcpipObj,'SOUR1:VOLT 1');               % Set amplitude of output signal


fprintf(tcpipObj,'SOUR1:FREQ:FIX 5');           % Set a slow frequency of output signal

fprintf(tcpipObj,'SOUR1:TRIG:SOUR GATED');      % Enter gated burst mode. This sets delaut parameters to burst mode
fprintf(tcpipObj,'SOUR1:BURS:NCYC 3');          % Set number of generated signals in one burst

fprintf(tcpipObj,'OUTPUT1:STATE ON'); 

fclose(tcpipObj);
