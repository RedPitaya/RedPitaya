%% Define Red Pitaya as TCP/IP object
clc
close all
% IP= '192.168.101.108';          % IP of your Red Pitaya...
IP= 'rp-f01b63.local';          % rp-MAC.local MAC are the last 6 characters of your Red Pitaya
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port);      

fopen(RP);
RP.Terminator = 'CR/LF';

%% SCPI exchange

% set output amplitude and offset
fprintf(RP,'SOURce1:VOLTage:IMMediate:AMPlitude 1');
fprintf(RP,'SOURce2:VOLTage:IMMediate:AMPlitude 1');
fprintf(RP,'SOURce1:VOLTage:IMMediate:OFFSet 0');
fprintf(RP,'SOURce2:VOLTage:IMMediate:OFFSet 0');

% specify peridic mode, sinusoidal waveform and 1kHZ frequency
fprintf(RP,'SOURce1:MODE PERiodic');
fprintf(RP,'SOURce2:MODE PERiodic');
fprintf(RP,'SOURce1:FUNCtion:SHAPe SINusoid');
fprintf(RP,'SOURce2:FUNCtion:SHAPe SINusoid');
fprintf(RP,'SOURce1:FREQuency:FIXed 20000');
fprintf(RP,'SOURce2:FREQuency:FIXed 40000');
fprintf(RP,'SOURce1:PHASe:ADJust 0');
fprintf(RP,'SOURce2:PHASe:ADJust 90');

% both generator should be synchronously driven
fprintf(RP,'SOURce1:EVENT:SYNChronization:SOURce GEN0');
fprintf(RP,'SOURce2:EVENT:SYNChronization:SOURce GEN0');

% reset and start state machine
fprintf(RP,'SOURce:RESET');
fprintf(RP,'SOURce:START');

% enable output
fprintf(RP,'OUTPUT1:STATe ON');
fprintf(RP,'OUTPUT2:STATe ON');

% trigger state machine
fprintf(RP,'SOURce:TRIGger');

%% Close connection to the Red Pitaya
fclose(RP);
