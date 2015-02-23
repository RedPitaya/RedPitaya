
%% Define Red Pitaya as TCP/IP object
clear all
close all
clc
IP= '192.168.178.56';                % Input IP of your Red Pitaya...
port = 5000;                         % If you are using WiFi then IP is:                  
tcpipObj = tcpip(IP, port);          % 192.168.128.1
tcpipObj.InputBufferSize = 16384*32;

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

flushinput(tcpipObj);
flushoutput(tcpipObj);

% Set decimation vale (sampling rate) in respect to you 
% acquired signal frequency

fprintf(tcpipObj,'ACQ:DEC 8');

% Set trigger level to 100 mV

fprintf(tcpipObj,'ACQ:TRIG:LEV 100');

% Set trigger delay to 0 samples
% 0 samples delay set trigger to center of the buffer
% Signal on your graph will have trigger in the center (symmetrical)
% Samples from left to the center are samples before trigger 
% Samples from center to the right are samples after trigger

fprintf(tcpipObj,'ACQ:TRIG:DLY 0');

pause(0.1) % Wait for data writing

%% Start & Trigg
% Trigger source setting must be after ACQ:START
% Set trigger to source 1 positive edge


fprintf(tcpipObj,'ACQ:START');
fprintf(tcpipObj,'ACQ:TRIG CH1_PE');  
 
% Wait for trigger
% Until trigger is true wait with acquiring
% Be aware of while loop if trigger is not achieved
% Ctrl+C will stop code executing in Matlab  


while 1
     trig_rsp=query(tcpipObj,'ACQ:TRIG:STAT?')
   
     if strcmp('TD',trig_rsp(1:2))  % Read only TD
   
     break
   
     end
 end
 
 
% Read data from buffer 

signal_str=query(tcpipObj,'ACQ:SOUR1:DATA?');

% Convert values to numbers.% First character in string is “{“   
% and 2 latest are empty spaces and last is “}”.  

signal_num=str2num(signal_str(1,2:length(signal_str)-3));

plot(signal_num)
grid on

%For plotting signal in respect to time you can use code below

Fs=str2num(query(tcpipObj,'ACQ:SRA:HZ?'));
dec=str2num(query(tcpipObj,'ACQ:DEC?'));
buffer_ln=16384;
%Create time vector in respect to                
%decimation value
t=0:1/(Fs/dec):1/(Fs/dec)*(buffer_ln-1); 
%plot(t,signal_num);
%grid on

%Reset to deafault values

fprintf(tcpipObj,'ACQ:RST');

fclose(tcpipObj)
