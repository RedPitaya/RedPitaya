%% Define Red Pitaya as TCP/IP object
clc
close all
% IP= '192.168.101.108';          % IP of your Red Pitaya...
IP= 'rp-f01b63.local';          % rp-MAC.local MAC are the last 6 characters of your Red Pitaya
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port);       

fopen(RP);
RP.Terminator = 'CR/LF';

% specify peridic mode, sinusoidal waveform and 1kHZ frequency
fprintf(RP,'SOURce1:MODE PERiodic');           % periodic 
fprintf(RP,'SOURce1:FUNCtion:SHAPe SINusoid'); % select signal shape as sinusoid
fprintf(RP,'SOURce1:FREQuency:FIXed 10000');    % Set frequency of output signal to 3kHz

% set output amplitude and offset
fprintf(RP,'SOURce1:VOLTage:IMMediate:AMPlitude 1');  % Set amplitude of the output signal
fprintf(RP,'SOURce1:VOLTage:IMMediate:OFFSet 0');     % Set offset of the output signal

% reset and start state machine
fprintf(RP,'SOURce1:RESET');
fprintf(RP,'SOURce1:START');

% enable output
fprintf(RP,'OUTPUT1:STATe ON');

% trigger state machine
fprintf(RP,':SOURce1:TRIGger');

%% Close connection to the Red Pitaya
fclose(RP);
