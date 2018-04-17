%% Define Red Pitaya as TCP/IP object
clc
close all
% IP= '192.168.101.108';          % IP of your Red Pitaya...
IP= 'rp-f01b63.local';          % rp-MAC.local MAC are the last 6 characters of your Red Pitaya
port = 5000;                    % If you are using WiFi then IP is:
RP=tcpip(IP, port,'InputBufferSize',32784*6);      
fopen(RP);
RP.Terminator = 'CR/LF';

%% SCPI exchange
% select data transmittion BIN/ASCII (0 - binary, any other value - ASCII)
bin=1;
% set buffer size on Red Pitaya
buffer_size = 2^14;


% wait for data to be captured by RedPitaya
while( str2double(query(RP,'ACQuire1:RUN?')) )
    continue
end
fprintf ('triggered\n');

%% read back table data
% if bin
    % read ch 1
    fprintf(RP,'ACQuire1:TRACe:DATA:RAW? 16000 ');
    data1 = binblockread(RP,'int16');
    data1 = data1 ./ 2^15;
    
    % read ch 2
    fprintf(RP,'ACQuire2:TRACe:DATA:RAW? 16000 ');
    data2 = binblockread(RP,'int16');
    data2 = data2 ./ 2^15;
plot(data1);
hold on;
plot(data2,'r');

% else
    
    string1= query(RP,'ACQuire1:TRACe:DATA:DATA? 16000 ');
    string2= query(RP,'ACQuire2:TRACe:DATA:DATA? 16000 ');
    % convert string to numbers
    data1=str2num(string1);
    data2=str2num(string2);
% end 

% plot gethered data
figure();
plot(data1);
hold on;
plot(data2,'r');

%% Close connection to the Red Pitaya
fclose(RP);