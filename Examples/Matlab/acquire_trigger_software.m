
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

%% Clean buffer

flushinput(tcpipObj);
flushoutput(tcpipObj);

%% Start & Trigg
% Trigger source setting must be after ACQ:START
% Set trigger to source 1 positive edge

pause(0.2) % wait for data writing

fprintf(tcpipObj,'ACQ:START');
fprintf(tcpipObj,'ACQ:TRIG NOW');  

% Check trigger
% If trigger is TD , trigger is achived
% Trig NOW set trigger true 

query(tcpipObj,'ACQ:TRIG:STAT?')
 
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

