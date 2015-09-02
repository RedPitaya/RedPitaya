%lcr pitaya test matlab
%created by martin and zumy  / 11.8.2014
% version 1.1

clear all;
close all;
clc

program = './lcr';
chanel = '1';
apmlitude = '1';
DC_bias = '0';
R_shutnt = '996';
averaging = '1';
calib_function = '0';
Z_load_ref_real = '0';
Z_load_ref_imag = '0';
steps = '100';
sweep_func = '1';
start_freq = '6400';
stop_freq = '6600';
scale_type = '0';
wait = '0';

user = 'root';
ip = '192.168.81.73';


command=['"C:\Users\zumrett\Desktop\plink" -l root -pw root ',ip,' "/opt/bin/lcr ',chanel,' ',apmlitude,' ',DC_bias,' ',R_shutnt,' ', averaging,' ', calib_function,' ',Z_load_ref_real,' ',Z_load_ref_imag,' ',steps,' ', sweep_func,' ', start_freq,' ', stop_freq,' ', scale_type,' ', wait,'"'];

[c,data] = dos(command);
data=str2num(data);


if (str2num(sweep_func) == 1)
     data_X = (data(:,1))';
else 
     data_X = 1:length((data(:,1))');
end

Phase=(data(:,2))';
Z_amp_C=(data(:,3))';


figure
subplot(2,1,1)
plot(data_X,Z_amp_C,'-o')
% semilogx(data_X,Z_amp_C,'-o')  % for scale_type=1
ylabel('Z_amp/ Ohm ')
if (str2num(sweep_func) == 1)
xlabel(' Frequency / Hz')     
else
xlabel(' Measurments') 
end    
grid on

subplot(2,1,2)
plot(data_X,Phase,'-ro')
%semilogx(data_X,Phase,'-ro')   % for scale_type=1
ylabel('Phase ')
if (str2num(sweep_func) == 1)
xlabel(' Frequency / Hz')     
else
xlabel(' Measurments') 
end    
grid on