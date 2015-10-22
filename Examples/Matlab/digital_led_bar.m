%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';


        %% Define value p from 0 - 100 %
        p = 67;    % Set value of p

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