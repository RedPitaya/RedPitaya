%% Define Red Pitaya as TCP/IP object
IP= '192.168.178.108';          % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

fprintf(tcpipObj,'ANALOG:PIN AOUT0,0.3');  % 0.3 Volts is set on output 0
fprintf(tcpipObj,'ANALOG:PIN AOUT1,0.9');
fprintf(tcpipObj,'ANALOG:PIN AOUT2,1');
fprintf(tcpipObj,'ANALOG:PIN AOUT3,1.5');

fclose(tcpipObj);