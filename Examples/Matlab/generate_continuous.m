%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

fprintf(tcpipObj,'GEN:RST');               % Reset generator

fprintf(tcpipObj,'SOUR1:FUNC SINE');       % Set function of output signal
                                           % {sine, square, triangle, sawu,sawd, pwm}
fprintf(tcpipObj,'SOUR1:FREQ:FIX 1000');   % Set frequency of output signal
fprintf(tcpipObj,'SOUR1:VOLT 1');          % Set amplitude of output signal

fprintf(tcpipObj,'OUTPUT1:STATE ON');      % Set output to ON
fprintf(tcpipObj,'SOUR1:TRIG:INT');        % Generate trigger

%% Close connection with Red Pitaya

fclose(tcpipObj);
