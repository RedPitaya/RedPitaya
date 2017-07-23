%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

%% Send SCPI command to Red Pitaya to turn ON LED1

fprintf(tcpipObj,'DIG:PIN LED1,1');

pause(5)                         % Set time of LED ON

%% Send SCPI command to Red Pitaya to turn OFF LED1

fprintf(tcpipObj,'DIG:PIN LED1,0');

%% Close connection with Red Pitaya

fclose(tcpipObj);