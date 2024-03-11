%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
fprintf(tcpipObj,'CAN:FPGA ON');

fprintf(tcpipObj,'CAN0:STOP');             % stop can interface for configure

fprintf(tcpipObj,'CAN0:BITRate 200000');   % set bitrate for can0
res = query(tcpipObj,'CAN0:BITRate:SP?');
fprintf('Bitrate %s\n', res);

fprintf(tcpipObj,'CAN0:MODE LOOPBACK,ON'); % set loopback mode

fprintf(tcpipObj,'CAN0:START');            % start can0 interface 

fprintf(tcpipObj,'CAN0:OPEN');             % open can0 socket 

fprintf(tcpipObj,'CAN0:Send123 1,2,3');    % write to can0 3 bytes
fprintf('CAN0:Send123 1,2,3\n');

res = query(tcpipObj,'CAN0:Read:Timeout2000?'); % read frame from can0
fprintf('Read: %s', res);

fprintf(tcpipObj,'CAN0:CLOSE');            % close can0

%% Close connection with Red Pitaya

fclose(tcpipObj);