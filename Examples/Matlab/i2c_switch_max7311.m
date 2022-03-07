%% Define Red Pitaya as TCP/IP object

IP= '';           % Input IP of your Red Pitaya...
port = 5000;
tcpipObj=tcpip(IP, port);

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

% working with RP 250-12 v1.2. For RP version 1.1 need replace dev address to 32 (0x20)

fprintf(tcpipObj,'I2C:DEV33 "/dev/i2c-0"');

fprintf(tcpipObj,'I2C:FMODE ON');          % set force mode

fprintf('Turn on AC/DC ch1 & ch2\n');

value = 0x55;
fprintf(tcpipObj,sprintf('I2C:S:W2 %d',value)); % write 2 bytes in i2c throw SMBUS

java.lang.Thread.sleep(1000);

value = value & ~ 0x0F;
fprintf(tcpipObj,sprintf('I2C:S:W2 %d',value)); % write 2 bytes in i2c throw SMBUS

java.lang.Thread.sleep(3000);

fprintf('Turn off AC/DC ch1 & ch2\n');

value = 0xAA;
fprintf(tcpipObj,sprintf('I2C:S:W2 %d',value)); % write 2 bytes in i2c throw SMBUS

java.lang.Thread.sleep(1000);

value = value & ~ 0x0F;
fprintf(tcpipObj,sprintf('I2C:S:W2 %d',value)); % write 2 bytes in i2c throw SMBUS

java.lang.Thread.sleep(1000);

fprintf(tcpipObj,'I2C:S:R2'); % read 2 bytes from reg 0x02 throw SMBUS
value = str2num(query(tcpipObj,'I2C:S:R2'));
fprintf('Reg 0x02: %x\n',value);


%% Close connection with Red Pitaya

fclose(tcpipObj);
