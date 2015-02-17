%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:                  
tcpipObj=tcpip(IP, port);       % 192.168.128.1

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

% Here you can use for loop and pause for generating train of pulses
% for i=1:1:1000    
fprintf(tcpipObj,'SOUR1:FUNC SINE');       % Set function of output signal     
                                           % {sine, square, triangle,                       
                                           % sawu,sawd, pwm}

fprintf(tcpipObj,'SOUR1:FREQ:FIX 10');     % Set frequency of output signal                
fprintf(tcpipObj,'SOUR1:VOLT 1');          % Set amplitude of output signal

fprintf(tcpipObj,'SOUR1:BURS:NCYC 1');     % Set 1 pulses of sine wave 
fprintf(tcpipObj,'SOUR1:TRIG:IMM');        % Set generator trigger to                               
                                           % immediately 
fprintf(tcpipObj,'OUTPUT1:STATE ON');      % Set output to ON 
fprintf(tcpipObj,'SOUR1:BURS:STAT ON');    % Set burst mode to ON
% pause(0.2)
% end
%% Close connection with Red Pitaya

fclose(tcpipObj);
