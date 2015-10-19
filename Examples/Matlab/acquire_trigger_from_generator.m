%% Define Red Pitaya as TCP/IP object
clc
clear all
close all

IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:                  
tcpipObj=tcpip(IP, port);       % 192.168.128.1
tcpipObj.InputBufferSize = 16384*32;
tcpipObj.OutputBufferSize = 16384*32;

%% Open connection with your Red Pitaya
fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
flushinput(tcpipObj)
flushoutput(tcpipObj)

%% Generate

fprintf(tcpipObj,'SOUR1:FUNC SINE');          % Set function of output signal {sine, square, triangle,sawu,sawd, pwm}
fprintf(tcpipObj,'SOUR1:FREQ:FIX 50000');     % Set frequency of output signal                
fprintf(tcpipObj,'SOUR1:VOLT 1');             % Set amplitude of output signal
fprintf(tcpipObj,'SOUR1:BURS:NCYC 1');        % Set 1 pulses of sine wave 
fprintf(tcpipObj,'OUTPUT1:STATE ON');         % Set output to ON 
fprintf(tcpipObj,'SOUR1:BURS:STAT ON');       % Set burst mode to ON
 
%% Set Acquire

fprintf(tcpipObj,'ACQ:DEC 1');
fprintf(tcpipObj,'ACQ:TRIG:LEV 50');
fprintf(tcpipObj,'ACQ:TRIG:DLY 0');

pause(0.01)

%% Start gen % acq

fprintf(tcpipObj,'ACQ:START');
fprintf(tcpipObj,'ACQ:TRIG AWG_PE'); 
fprintf(tcpipObj,'SOUR1:TRIG:IMM');           % Set generator trigger to immediately 

%% Wait for trigger
 while 1
    trig_rsp=query(tcpipObj,'ACQ:TRIG:STAT?')
    if strcmp('TD',trig_rsp(1:2))
    break
    end
 end
  
%% Read & plot

signal_str=query(tcpipObj,'ACQ:SOUR1:DATA?');
signal_num=str2num(signal_str(1,2:length(signal_str)-3));
plot(signal_num)
hold on
grid on

%% Close connection with Red Pitaya
fprintf(tcpipObj,'ACQ:RST');
fclose(tcpipObj);