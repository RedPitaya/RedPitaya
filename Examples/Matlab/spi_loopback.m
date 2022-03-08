%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
fprintf(tcpipObj,'SPI:INIT:DEV "/dev/spidev1.0"');

fprintf(tcpipObj,'SPI:SET:DEF');           % set default settings

fprintf(tcpipObj,'SPI:SET:GET');           % get default settings

fprintf(tcpipObj,'SPI:SET:MODE LIST');     % set mode: Low idle level, sample on trailing edge

fprintf('Mode %s\n', query(tcpipObj,'SPI:SET:MODE?')); % check current mode setting

fprintf(tcpipObj,'SPI:SET:SPEED 5000000'); % set spi speed

fprintf('Speed %s\n', query(tcpipObj,'SPI:SET:SPEED?')); % check current speed setting

fprintf(tcpipObj,'SPI:SET:WORD 8');        % set word length

fprintf('Word length %s\n', query(tcpipObj,'SPI:SET:WORD?')); % check current speed setting

fprintf(tcpipObj,'SPI:SET:SET');           % apply setting to spi

%% Work with spi messages

fprintf(tcpipObj,'SPI:MSG:CREATE 2');      % create 2 messages with diffrent buffers

fprintf('Check message count %s\n', query(tcpipObj,'SPI:MSG:SIZE?')); 

fprintf(tcpipObj,'SPI:MSG0:TX4:RX 13,14,15,16');  % sets the first message to write and read buffers of 4 bytes

fprintf(tcpipObj,'SPI:MSG1:RX7:CS'); % Sets the buffer for the second message to read 7 bytes long and switch the CS signal level

fprintf(tcpipObj,'SPI:PASS'); % sends data to SPI

fprintf('TX buffer of 1 msg %s\n', query(tcpipObj,'SPI:MSG0:TX?'));

fprintf('RX buffer of 1 msg %s\n', query(tcpipObj,'SPI:MSG0:TX?'));

fprintf('RX buffer of 2 msg %s\n', query(tcpipObj,'SPI:MSG1:RX?'));

fprintf(tcpipObj,'SPI:MSG:DEL'); % Deletes messages


%% Close connection with Red Pitaya

fprintf(tcpipObj,'SPI:RELEASE');           % close spi

fclose(tcpipObj);
