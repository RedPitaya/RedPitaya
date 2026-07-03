%lcr pitaya test octave
%created by martin and zumy  / 11.8.2014
% version 1.1

clear all;
close all;
clc

chanel = '1';
apmlitude = '1';
DC_bias = '0';
shunt = '999';
averaging = '5';
calib_fnc = '0';
ref_imp_real = '0';
ref_imp_imag = '0';
steps = '100';
sweep_fnc = '1';
start_freq = '200';
stop_freq = '100000';
scale_type = '1';
wait = '0';

user = 'root';
ip = '192.168.81.73';

command=['ssh root@',ip,' "./lcr ',chanel,' ',apmlitude,' ',DC_bias,' ',shunt,' ',averaging,' ',calib_fnc,' ',ref_imp_real,' ',ref_imp_imag,' ',steps,' ',sweep_fnc,' ',start_freq,' ',stop_freq,' ',scale_type,' ',wait,'"'];

[c,data] = system(command);

data=str2num(data);


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
