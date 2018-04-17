%% Define Red Pitaya as TCP/IP object
clc
close all
% IP= '192.168.101.108';          % IP of your Red Pitaya...
IP= 'rp-f01b63.local';          % rp-MAC.local MAC are the last 6 characters of your Red Pitaya
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port,'InputBufferSize',32784*5);      
fopen(RP);
RP.Terminator = 'CR/LF';

%% SCPI exchange
% select data transmittion BIN/ASCII (0 - binary, any other value - ASCII)
bin=1;
% set buffer size on Red Pitaya
buffer_size = 2^14;

% data rate decimation 
fprintf(RP,'ACQuire1:INPut:DECimation 10');

% trigger timing [sample periods]
fprintf(RP,'ACQuire1:SAMPle:PRE 0');
fprintf(RP,'ACQuire1:SAMPle:POST %d ', buffer_size);
fprintf(RP,''); % fprintf does not complete with /n, hence one is added here 

% define event synchronization source
fprintf(RP,'ACQuire1:EVENT:SYNChronization:SOURce OSC1');
% disable hardware trigger sources
fprintf(RP,'ACQuire1:EVENT:TRIGger:SOURce NONE');

% synchronization source is the default, which is the module itself
% reset and start state machine
fprintf(RP,'ACQuire1:RESET');
fprintf(RP,'ACQuire1:START');
fprintf(RP,'ACQuire1:TRIGger');

% wait for data to be captured by RedPitaya
while( str2double(query(RP,'ACQuire1:RUN?')) )
    continue
end
fprintf ('triggered\n');

%% read back table data
% if bin
    fprintf(RP,'ACQuire1:TRACe:DATA:RAW? 16000 ');
    data1 = binblockread(RP,'int16');
    data1 = data1 ./ 2^15;
plot(data1)

% else
    
    string= query(RP,'ACQuire1:TRACe:DATA:DATA? 16000 ');
    % convert string to numbers
    data=str2num(string);
% end 

% plot gethered data
figure();
plot(data)

%% Close connection to the Red Pitaya
fclose(RP);