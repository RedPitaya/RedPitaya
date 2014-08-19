%bode pitaya test octave
%created by martin and zumy  / 11.8.2014

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
ip = '192.168.81.161';



command=['ssh root@',ip,' "/opt/bin/bode ',chanel,' ',apmlitude,' ',DC_bias,' ',averaging,' ',steps,' ',start_freq,' ',stop_freq,' ',scale_type,'"'];

[c,data] = system(command);

data=str2num(data);

figure
subplot(2,1,1)
semilogx(data(:,1),data(:,3),'-ro')
grid on

subplot(2,1,2)
semilogx(data(:,1),data(:,2),'-o')
grid on
