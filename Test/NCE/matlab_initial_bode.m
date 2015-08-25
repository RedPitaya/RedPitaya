%bode pitaya test matlab
%created by martin and zumy  / 11.8.2014
% version 1.1

clear all;
close all;
clc

chanel = '1';
apmlitude = '1';
DC_bias = '0';
averaging = '1';
steps = '100';
start_freq = '200';
stop_freq = '100000';
scale_type = '1';

user = 'root';

ip = '192.168.81.73'; %change to your ip

command=['"C:\Path\to\plink" -l root -pw root ',ip,' "./bode ',chanel,' ',apmlitude,' ',DC_bias,' ',averaging,' ',steps,' ',start_freq,' ',stop_freq,' ',scale_type,'"'];

[c,data] = dos(command);

data=str2num(data);

%depending on scale type graphs have linear or logarithmic scale
if (str2num(scale_type)) == 0 
	figure
	subplot(2,1,1)
	plot(data(:,1),data(:,3),'-ro')
	ylabel('Z_amp/ Ohm ')
	xlabel(' Measurments')
	grid on

	subplot(2,1,2)
	plot(data(:,1),data(:,2),'-o')
	ylabel('Phase ')
	xlabel(' Measurments')
	grid on

elseif (str2num(scale_type)) == 1 
	figure
	subplot(2,1,1)
	semilogx(data(:,1),data(:,3),'-ro')
	ylabel('Z_amp/ Ohm ')
	xlabel(' Frequency / Hz') 
	grid on

	subplot(2,1,2)
	semilogx(data(:,1),data(:,2),'-o')
	ylabel('Phase ')
	xlabel(' Frequency / Hz')
	grid on
end;
