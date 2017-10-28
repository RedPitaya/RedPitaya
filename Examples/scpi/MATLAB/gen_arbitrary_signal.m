% %% Define Red Pitaya as TCP/IP object
clc
close all
% IP= '192.168.101.108';          % IP of your Red Pitaya...
IP= 'rp-f01b63.local';          % rp-MAC.local MAC are the last 6 characters of your Red Pitaya
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port,'OutputBufferSize',32784*5);      
fopen(RP);
RP.Terminator = 'CR/LF';

%% Prepare an arbitrary waveform

% select data transmittion BIN/ASCII (0 - binary, any other value - ASCII)
bin=1;
% select buffer length (max = 2^14)
length = 2^3;
% STEMlab 14 - 1 sign + 13 data bits
bitsize= 0;

% generates x axis in range 0 to 6 with length number of points
X0=0;
XN=length-1;
x  = X0:(XN-X0)/(length-1):XN ;
% Values of arbitrary waveform must be in range from -1 to 1.
y1 = 0.8 * sin(x); % the first sinus signal with the amplitude 0.8
y2 = 0.2 * sin(21*x); % the second sinus signal with a frequency 20 times higher than the first one and the amplitude of 0.2
% y2 = 0.2 * sin(5*x); % the second sinus signal with a frequency 20 times higher than the first one and the amplitude of 0.2
y_sum = x;

% uncomment the following line to show the plot of the arbitrary function
plot(x,y_sum);

%% transmit data and configure genertor

% set output amplitude and offset
fprintf(RP,'SOURce1:VOLTage:IMMediate:AMPlitude 1');
fprintf(RP,'SOURce1:VOLTage:IMMediate:OFFSet 0');

% specify peridic mode
fprintf(RP,'SOURce1:MODE PERiodic');

%%% transmit arbitrary waveform in either BINARY or ASCII format
if bin
    disp('INFO: Sending data in BINARY format');
    binblockwrite(RP, y_sum * (2^bitsize),'int16','SOURce1:TRACe:DATA:RAW ');
    fprintf(RP,''); %binblockwrite does not complete with /n, hence one is added here
    
    y_sum * (2^bitsize)
    fprintf(RP,'SOURce1:TRACe:DATA:RAW? 8 ');
    data1 = binblockread(RP,'int16')
else
    disp('INFO: Sending data in ASCII format');
    % convert float to string
    waveform_ch_1_0 = num2str(y_sum,'%1.5f,');
    % remove the last “,”.
    waveform_ch_1 = waveform_ch_1_0(1,1: size(waveform_ch_1_0,2)-1);
    fprintf(RP,['SOURce1:TRACe:DATA:DATA ' waveform_ch_1]);
end

% set 1kHZ frequency
fprintf(RP,'SOURce1:FREQuency:FIXed 2000');

%  reset and start state machine
fprintf(RP,'SOURce1:RESET');
fprintf(RP,'SOURce1:START');

% # enable output
fprintf(RP,'OUTPUT1:STATe ON');

% # trigger state machine
fprintf(RP,'SOURce1:TRIGger');

% RA = binblockread(RP,'int16');

%% Close connection to the Red Pitaya
fclose(RP);

