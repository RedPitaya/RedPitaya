clc
clear all
close all

IP= '192.168.178.111';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);
tcpipObj.InputBufferSize = 16384*32;
tcpipObj.OutputBufferSize = 16384*32;

%% Open connection with your Red Pitaya
fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
flushinput(tcpipObj)
flushoutput(tcpipObj)

%% Loop back for testing Generate 

%% The example generate sine bursts every 0.5 seconds indefinety
fprintf(tcpipObj,'GEN:RST');
fprintf(tcpipObj,'ACQ:RST');

fprintf(tcpipObj,'SOUR1:FUNC SINE');                                                 
fprintf(tcpipObj,'SOUR1:FREQ:FIX 1000');     % Set frequency of output signal
fprintf(tcpipObj,'SOUR1:VOLT 1');          % Set amplitude of output signal

fprintf(tcpipObj,'SOUR1:BURS:STAT ON');    % Set burst mode to ON
fprintf(tcpipObj,'SOUR1:BURS:NCYC 1');       % Set 1 pulses of sine wave
fprintf(tcpipObj,'OUTPUT1:STATE ON');         % Set output to ON

%% Set Acquire

fprintf(tcpipObj,'ACQ:DEC 64');
fprintf(tcpipObj,'ACQ:TRIG:LEV 0');
fprintf(tcpipObj,'ACQ:TRIG:DLY 0');


%% Start gen % acq

fprintf(tcpipObj,'ACQ:START');
pause(1);
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
fclose(tcpipObj);
