%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
fprintf(tcpipObj,'UART:INIT');

fprintf(tcpipObj,'UART:BITS CS7');         % set size 7 bit
res = query(tcpipObj,'UART:BITS?');        % check current settings for bit size 
fprintf('Bit size %s\n', res);

fprintf(tcpipObj,'UART:SPEED 57600');      % set uart speed
res = query(tcpipObj,'UART:SPEED?');       % check current settings for speed
fprintf('Speed %s\n', res);

fprintf(tcpipObj,'UART:STOPB STOP2');      % set stop bits
res = query(tcpipObj,'UART:STOPB?');       % check current settings for stop bits
fprintf('Stop bits %s\n', res);

fprintf(tcpipObj,'UART:PARITY ODD');       % set parity
res = query(tcpipObj,'UART:PARITY?');      % check current settings for parity
fprintf('Parity %s\n', res);

fprintf(tcpipObj,'UART:TIMEOUT 10');       % set timeout in 1/10 sec. 10 = 1 sec 
res = query(tcpipObj,'UART:TIMEOUT?');     % check current settings for parity
fprintf('Timeout %s\n', res);

fprintf(tcpipObj,'UART:SETUP');           % apply setting to uart 

fprintf(tcpipObj,'UART:WRITE7 #H11,#H22,#H33,33,33,#Q11,#B11001100');  % write to uart 7 bytes
fprintf('Write 7 bytes to uart: #H11,#H22,#H33,33,33,#Q11,#B11001100\n');

res = query(tcpipObj,'UART:READ3');        % read from uart 3 bytes
fprintf('Read: %s\n', res);

res = query(tcpipObj,'UART:READ4');        % read from uart 4 bytes
fprintf('Read: %s\n', res);

fprintf(tcpipObj,'UART:RELEASE');          % close uart

%% Close connection with Red Pitaya

fclose(tcpipObj);