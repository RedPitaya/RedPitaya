%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:               
tcpipObj=tcpip(IP, port);       % 192.168.128.1

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';


fprintf(tcpipObj,'DIG:PIN:DIR INP,DIO5_N'); % Set DIO5_N  to be input

i=1;

while i<1000                    			% You can set while 1 for continuous loop 
    
state=str2num(query(tcpipObj,'DIG:PIN? DIO5_N'));

    if state==1

      fprintf(tcpipObj,'DIG:PIN LED5,0');
   
    end

    if state==0

      fprintf(tcpipObj,'DIG:PIN LED5,1');

    end

pause(0.1)                     				% Set time delay for Red Pitaya response 

i=i+1

end    

%% Close connection with Red Pitaya

fclose(tcpipObj);
