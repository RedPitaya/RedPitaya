%% Define Red Pitaya as TCP/IP object
clc
close all
% IP= '192.168.101.108';          % IP of your Red Pitaya...
IP= 'rp-f01b63.local';          % rp-MAC.local MAC are the last 6 characters of your Red Pitaya
port = 5000;              % If you are using WiFi then IP is:
RP=tcpip(IP, port);      

fopen(RP);
RP.Terminator = 'CR/LF';

%% The example generates 4 bursts of half a sine every 3/4 of sine period
buffer_size = 2^14;

% set output amplitude and offset
fprintf(RP,'SOURce1:VOLTage:IMMediate:AMPlitude 1');  % Set amplitude of the output signal
fprintf(RP,'SOURce1:VOLTage:IMMediate:OFFSet 0');     % Set offset of the output signal

% specify peridic mode, sinusoidal waveform
fprintf(RP,'SOURce1:MODE BURSt');
fprintf(RP,'SOURce1:BURSt:MODE FINite');
fprintf(RP,'SOURce1:FUNCtion:SHAPe SINusoid');

% burst half the buffer with then idle for quarter buffer, repeat 4 times
fprintf(RP,'SOURce1:BURSt:DATA:REPetitions 1');
fprintf(RP,['SOURce1:BURSt:DATA:LENgth '   , num2str(1 * buffer_size / 2,'%1.5f ')]);
fprintf(RP,['SOURce1:BURSt:PERiod:LENgth ' , num2str(3 * buffer_size / 4,'%1.5f ')]);
fprintf(RP,'SOURce1:BURSt:PERiod:NUMber 4');

% reset and start state machine
fprintf(RP,'SOURce1:RESET');
fprintf(RP,'SOURce1:START');

% enable output
fprintf(RP,'OUTPUT1:STATe ON');

% trigger state machine
fprintf(RP,'SOURce1:TRIGger');

%% Close connection to the Red Pitaya
fclose(RP);

