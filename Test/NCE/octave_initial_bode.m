%bode pitaya test octave
%created by martin and zumy  / 11.8.2014
% version 1.1

clear all;
close all;
clc

chanel = '1';
apmlitude = '1';
DC_bias = '0';
averaging = '3';
steps = '100';
start_freq = '100';
stop_freq = '80000';
scale_type = '1';

user = 'root';

ip = '192.168.81.73';

command=['ssh root@',ip,' "./bode ',chanel,' ',apmlitude,' ',DC_bias,' ',averaging,' ',steps,' ',start_freq,' ',stop_freq,' ',scale_type,'"'];

[c,data] = system(command);

data=str2num(data);

ip = '192.168.81.161';%change to your ip

%depending on scale type graphs have linear or logarithmic scale
if (str2num(scale_type)) == 0 
	figure
	subplot(2,1,1)
	plot(data(:,1),data(:,3),'-ro')
	grid on

	subplot(2,1,2)
	plot(data(:,1),data(:,2),'-o')
	grid on

elseif (str2num(scale_type)) == 1 
	figure
	subplot(2,1,1)
	semilogx(data(:,1),data(:,3),'-ro')
	grid on

	subplot(2,1,2)
	semilogx(data(:,1),data(:,2),'-o')
	grid on
end;
