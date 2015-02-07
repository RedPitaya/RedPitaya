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

fprintf(tcpipObj,'GEN:RST')                       % Reset to default settings

fprintf(tcpipObj,'SOUR1:FREQ:FIX 5');             % Set a slow frequency of output signal

fprintf(tcpipObj,'SOUR1:BURS:NCYC 3');            % Set number of generated signals in one burst
fprintf(tcpipObj,'SOUR1:BURS:NOR 5');             % Set number of repeated bursts
fprintf(tcpipObj,'SOUR1:BURS:INT:PER 1000000');   % Set time of burst period in microseconds = 6 * 1/Frequency * 1000000
                                                  % Time must be greater than 1/Frequency * burst count (BURS:NCYC)
                                                  
fprintf(tcpipObj,'OUTPUT1:STATE ON'); 

fclose(tcpipObj);

