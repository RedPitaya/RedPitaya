%% Define Red Pitaya as TCP/IP object

IP= '192.168.178.108';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

volts0=str2num(query(tcpipObj,'ANALOG:PIN? AIN0'))
volts1=str2num(query(tcpipObj,'ANALOG:PIN? AIN1'))
volts2=str2num(query(tcpipObj,'ANALOG:PIN? AIN2'))
volts3=str2num(query(tcpipObj,'ANALOG:PIN? AIN3'))

%% Close connection with Red Pitaya

fclose(tcpipObj);