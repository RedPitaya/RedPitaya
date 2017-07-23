%%Define Red Pitaya as TCP/IP object
clc
clear all
close all
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);
tcpipObj.InputBufferSize = 16384*64;
tcpipObj.OutputBufferSize = 16384*64;
flushinput(tcpipObj)
flushoutput(tcpipObj)

%% Open connection with your Red Pitaya
x=instrfind;
fclose(x);
fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

%% Import arbitrary waveform
% Donwload ch1.csv and ch2.csv and copy them to
% your Matlab/bin folder
t=1:1:16384;
x=dlmread('batman_ch1.csv')';
y=dlmread('batman_ch2.csv')';
plot(t,x,t,y)
grid on

%% Convert waveforms to string with 5 decimal places accuracy
waveform_ch_1_0 =num2str(x,'%1.5f,');
waveform_ch_2_0 =num2str(y,'%1.5f,');

% latest is “,”.  
waveform_ch_1 =waveform_ch_1_0(1,1:length(waveform_ch_1_0)-1);
waveform_ch_2 =waveform_ch_2_0(1,1:length(waveform_ch_2_0)-1);

%%
fprintf(tcpipObj,'GEN:RST')                     % Reset to default settings

fprintf(tcpipObj,'SOUR1:FUNC ARBITRARY');       % Set function of output signal
fprintf(tcpipObj,'SOUR2:FUNC ARBITRARY');       % {sine, square, triangle,sawu,sawd,pwm}

fprintf(tcpipObj,['SOUR1:TRAC:DATA:DATA ' waveform_ch_1])
fprintf(tcpipObj,['SOUR2:TRAC:DATA:DATA ' waveform_ch_2])

fprintf(tcpipObj,'SOUR1:VOLT 1');               % Set amplitude of output signal
fprintf(tcpipObj,'SOUR2:VOLT 1');

fprintf(tcpipObj,'SOUR1:FREQ:FIX 5000');        % Set frequency of output signal
fprintf(tcpipObj,'SOUR2:FREQ:FIX 5000');

fprintf(tcpipObj,'OUTPUT1:STATE ON');
fprintf(tcpipObj,'OUTPUT2:STATE ON');


fclose(tcpipObj);