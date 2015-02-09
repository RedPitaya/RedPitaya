%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';           % Input IP of your Red Pitaya...
port = 5000;                    % If you are using WiFi then IP is:                  
tcpipObj=tcpip(IP, port);       % 192.168.128.1

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

% The example generate sine bursts every 0.5 seconds indefinety

fprintf(tcpipObj,'SOUR1:FUNC SINE');       % Set function of output signal     
                                           % {sine, square, triangle,                       
                                           % sawu,sawd, pwm}

fprintf(tcpipObj,'SOUR1:FREQ:FIX 10');     % Set frequency of output signal                
fprintf(tcpipObj,'SOUR1:VOLT 1');          % Set amplitude of output signal

fprintf(tcpipObj,'SOUR1:BURS:STAT ON');    % Set burst mode to ON
fprintf(tcpipObj,'SOUR1:BURS:NCYC 1');     % Set 1 pulses of sine wave
fprintf(tcpipObj,'SOUR1:BURS:NOR INF');    % Infinity number of sine wave pulses
fprintf(tcpipObj,'SOUR1:BURS:INT:PER 500000');     % Set time of burst period in microseconds = 5 * 1/Frequency * 1000000
fprintf(tcpipObj,'SOUR1:TRIG:IMM');        % Set generator trigger to immediately

fprintf(tcpipObj,'OUTPUT1:STATE ON');      % Set output to ON

%% Close connection with Red Pitaya

fclose(tcpipObj);
