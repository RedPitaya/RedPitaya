% %% Define Red Pitaya as TCP/IP object
clc
close all
IP= '192.168.178.103';          % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port,'OutputBufferSize',32784*5);       % 192.168.128.1

fopen(RP);
RP.Terminator = 'CR/LF';

%% Prepare an arbitrary waveform
% Values of arbitrary waveform must be in range from -1 to 1.
length = 2^14;

% generates x axis in range 0 to 6 with length number of points
X0=0;
XN=6;
x  = X0:(XN-X0)/(length-1):XN ;
y1 = 0.8 * sin(x); % the first sinus signal with the amplitude 0.8
y2 = 0.2 * sin(21*x); % the second sinus signal with a frequency 20 times higher than the first one and the amplitude of 0.2
y_sum = y1+y2;

% plot(x,y_sum);

%% transmit data and configure genertor

%%% transmit data in binary format
% binary write still needs to be debuged
% disp(y_sum)
% binblockwrite(RP, y_sum,'int16','SOURce1:TRACe:DATA:RAW ');
% fprintf(RP,'\n');

%%% transmit data in ascii format
% convert float to string
waveform_ch_1_0 = num2str(y_sum,'%1.5f,');
% remove the last “,”.
waveform_ch_1 = waveform_ch_1_0(1,1: size(waveform_ch_1_0,2)-3);

% set output amplitude and offset
fprintf(RP,'SOURce1:VOLTage:IMMediate:AMPlitude 1');
fprintf(RP,'SOURce1:VOLTage:IMMediate:OFFSet 0');

% # specify peridic mode, sinusoidal waveform and 1kHZ frequency
fprintf(RP,'SOURce1:MODE PERiodic');

fprintf(RP,['SOURce1:TRACe:DATA:DATA ' waveform_ch_1]);
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

%% Close connection to the Red Pitaya
fclose(RP);

