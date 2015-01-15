%% Define Red Pitaya as TCP/IP object
        
IP= '192.168.178.56';                % Input IP of your Red Pitaya...
port = 5000;                         % If you are using WiFi then IP is:                  
tcpipObj=tcpip(IP, port);            % 192.168.128.1
tcpipObj.InputBufferSize = 16384*32; % Set matlab buffer size

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

fprintf(tcpipObj,'ACQ:TRIG:LEV 100');  % Set trigger level
fprintf(tcpipObj,'ACQ:START');         % Start acquiring
fprintf(tcpipObj,'ACQ:TRIG CH1_PE');   % Set trigger to channel 1 and positive  edge

                                      
% Read 16384(all) samples from buffer from oldest to newest. 
% The oldest sample is first sample after trigger  

signal_str=query(tcpipObj,'ACQ:SOUR1:DATA:OLD:N? 16384'); 

% Convert values to numbers.% First character in string is “{“   
% and 2 latest are empty spaces and last is “}”.  

signal_num=str2num(signal_str(1,2:length(signal_str)-3)); 

plot(signal_num);                    % Plot acquired signals  

% For plotting signal in respect to time you can use code below
%
% Fs=125000000;
% dec=1;
% buffer_ln=16384;
% t=0:1/(Fs/dec):1/(Fs/dec)*(buffer_ln-1); % Create time vector in respect to                
                                           % decimation value
% plot(t,signal_num);
                                            
%% Close connection with Red Pitaya

fclose(tcpipObj);
