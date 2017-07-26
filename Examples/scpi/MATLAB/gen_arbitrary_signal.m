%% Define Red Pitaya as TCP/IP object
clc
close all
IP= '192.168.178.103';          % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port,'OutputBufferSize',32784*5);       % 192.168.128.1

fopen(RP);
RP.Terminator = 'CR/LF';

%% Calcualte arbitrary waveform with 16384 samples
% Values of arbitrary waveform must be in range from -1 to 1.
% N=16384;
N=4;
bitsize=13;%

t=0:(2*pi)/(N-1) :2*pi;
y=sin(t)+1/3*sin(6*t);
y=y.*0.75;
plot(t,y)

% set output amplitude and offset
fprintf(RP,'SOURce1:VOLTage:IMMediate:AMPlitude 1');
fprintf(RP,'SOURce1:VOLTage:IMMediate:OFFSet 0');

% # specify peridic mode, sinusoidal waveform and 1kHZ frequency
fprintf(RP,'SOURce1:MODE PERiodic');
yInt=y.*(2^bitsize);

waveform_ch_1_0 =num2str(y,'%1.5f,');

% latest are empty spaces  “,”.
waveform_ch_1 =waveform_ch_1_0(1,1:length(waveform_ch_1_0)-3);

disp(yInt)
binblockwrite(RP, yInt,'int16','SOURce1:TRACe:DATA:RAW ');
% fprintf(RP,['SOURce1:TRACe:DATA:DATA ' waveform_ch_1]);
fprintf(RP,'SOURce1:FREQuency:FIXed 1000');
% 
% # reset and start state machine
fprintf(RP,'SOURce1:RESET');
fprintf(RP,'SOURce1:START');
% 
% # enable output
fprintf(RP,'OUTPUT1:STATe ON');
% 
% # trigger state machine
fprintf(RP,'SOURce1:TRIGger');


% binblockread
fclose(RP);

disp('DONE')
