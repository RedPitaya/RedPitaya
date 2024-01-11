%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
fprintf(tcpipObj,'CAN:FPGA ON');

fprintf(tcpipObj,'CAN0:STOP');             % stop can0 interface for configure

fprintf(tcpipObj,'CAN0:BITRate 200000');   % set bitrate for can0

fprintf(tcpipObj,'CAN0:MODE LOOPBACK,OFF'); % set loopback mode off

fprintf(tcpipObj,'CAN1:STOP');             % stop can1 interface for configure

fprintf(tcpipObj,'CAN1:BITRate 200000');   % set bitrate for can1

fprintf(tcpipObj,'CAN1:MODE LOOPBACK,OFF'); % set loopback mode  off

fprintf(tcpipObj,'CAN0:START');            % start can0 interface 

fprintf(tcpipObj,'CAN0:OPEN');             % open can0 socket 

fprintf(tcpipObj,'CAN1:START');            % start can1 interface 

fprintf(tcpipObj,'CAN1:OPEN');             % open can1 socket 

fprintf(tcpipObj,'CAN0:Send123 1,2,3');    % write to can0 3 bytes
fprintf('CAN0:Send123 1,2,3\n');

fprintf(tcpipObj,'CAN0:Send321:E 1,2,3,4,5');    % write to can0 3 bytes
fprintf('CAN0:Send321:E 1,2,3,4,5\n');

res = query(tcpipObj,'CAN1:Read:Timeout2000?'); % read frame from can1
fprintf('Read: %s', res);

res = query(tcpipObj,'CAN1:Read?');         % read frame from can1
fprintf('Read: %s', res);

fprintf(tcpipObj,'CAN0:CLOSE');            % close can0

fprintf(tcpipObj,'CAN1:CLOSE');            % close can1

%% Close connection with Red Pitaya

fclose(tcpipObj);