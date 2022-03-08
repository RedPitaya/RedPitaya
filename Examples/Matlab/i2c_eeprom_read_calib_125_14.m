%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';
fprintf(tcpipObj,'I2C:DEV80 "/dev/i2c-0"');

fprintf(tcpipObj,'I2C:FMODE ON');          % set force mode

% Eeprom 24c64 supports reading only 32 bytes of data at a time and only works through IOCTL

fprintf(tcpipObj,'I2C:IO:W:B2 0,0'); % set read address = 0


b1 = query(tcpipObj,'I2C:IO:R:B32'); % read 32 bytes from iic 

b2 = query(tcpipObj,'I2C:IO:R:B16'); % read 16 bytes from iic

b_num = str2num(b1(1,2:length(b1)-3));

b_num(33:48) = str2num(b2(1,2:length(b2)-3));

calib = typecast(uint8(b_num),'int32');

fprintf('ADC Ch1 High %d\n',calib(3));
fprintf('ADC Ch2 High %d\n',calib(4));
fprintf('ADC Ch1 Low %d\n',calib(5));
fprintf('ADC Ch2 Low %d\n',calib(6));
fprintf('ADC Ch1 Low offset %d\n',calib(7));
fprintf('ADC Ch2 Low offset %d\n',calib(8));
fprintf('DAC Ch1 %d\n',calib(9));
fprintf('DAC Ch2 %d\n',calib(10));
fprintf('DAC Ch1 offset %d\n',calib(11));
fprintf('DAC Ch2 offset %d\n',calib(12));

%% Close connection with Red Pitaya

fclose(tcpipObj);