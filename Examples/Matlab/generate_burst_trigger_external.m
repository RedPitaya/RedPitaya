%% Define Red Pitaya as TCP/IP object
clc
clear all
close all

IP= '192.168.178.56';            % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);


%% Open connection with your Red Pitaya
fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
flushinput(tcpipObj)
flushoutput(tcpipObj)

%% Generate

fprintf(tcpipObj,'SOUR1:FUNC SINE');          % Set function of output signal {sine, square, triangle,sawu,sawd, pwm}
fprintf(tcpipObj,'SOUR1:FREQ:FIX 200');       % Set frequency of output signal
fprintf(tcpipObj,'SOUR1:VOLT 1');             % Set amplitude of output signal

fprintf(tcpipObj,'SOUR1:BURS:NCYC 1');        % Set 1 pulses of sine wave
fprintf(tcpipObj,'OUTPUT1:STATE ON');         % Set output to ON
fprintf(tcpipObj,'SOUR1:BURS:STAT ON');       % Set burst mode to ON
 
fprintf(tcpipObj,'SOUR1:TRIG:SOUR EXT_PE');   % Set generator trigger to external

% For generating signal pulses you trigger signal frequency must be less than
% frequency of generating signal pulses. If you have trigger signal frequency  
% higher than frequency of generating signal pulses
% on output you will get continuous  signal instead of pulses


fclose(tcpipObj);