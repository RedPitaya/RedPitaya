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

fprintf(tcpipObj,'GEN:RST');               % Reset to default settings

fprintf(tcpipObj,'SOUR1:FUNC SINE');       % Set function of output signal
                                           % {sine, square, triangle, sawu,sawd, pwm}
fprintf(tcpipObj,'SOUR1:FREQ:FIX 2000');   % Set frequency of output signal
fprintf(tcpipObj,'SOUR1:VOLT 1');          % Set amplitude of output signal

fprintf(tcpipObj,'SOUR2:FUNC SINE');       % Set function of output signal
					   % {sine, square, triangle, sawu,sawd, pwm}
fprintf(tcpipObj,'SOUR2:FREQ:FIX 2000');   % Set frequency of output signal
fprintf(tcpipObj,'SOUR2:VOLT 1');          % Set amplitude of output signal

fprintf(tcpipObj,'OUTPUT:STATE ON');       % Start two channels simultaneously
fprintf(tcpipObj,'SOUR:TRIG:INT');         % Generate triggers

%% Close connection with Red Pitaya

fclose(tcpipObj);
