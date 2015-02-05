%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:               
tcpipObj=tcpip(IP, port);       % 192.168.128.1

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

volts=str2num(query(tcpipObj,'ANALOG:PIN? AIN3'));  % Red value on analog           
                                                    % input 3

%% Define value p from 0 - 100 %
         
        p = volts *(100/3.3);    % Set value of p in respect to readed voltage
        
        if p >=(100/7)
        fprintf(tcpipObj,'DIG:PIN LED1,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED1,0') 
        end    
        
        if p >=(100/7)*2
        fprintf(tcpipObj,'DIG:PIN LED2,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED2,0') 
        end   
        
        if p >=(100/7)*3
        fprintf(tcpipObj,'DIG:PIN LED3,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED3,0') 
        end   
        
        if p >=(100/7)*4
        fprintf(tcpipObj,'DIG:PIN LED4,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED4,0') 
        end   
        
        if p >=(100/7)*5
        fprintf(tcpipObj,'DIG:PIN LED5,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED5,0') 
        end   
        
        if p >=(100/7)*6
        fprintf(tcpipObj,'DIG:PIN LED6,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED6,0') 
        end   
        
        if p >=(100/7)*7
        fprintf(tcpipObj,'DIG:PIN LED7,1')        
        else
        fprintf(tcpipObj,'DIG:PIN LED7,0') 
        end   
        
        fclose(tcpipObj); 

%% Close connection with Red Pitaya

fclose(tcpipObj);