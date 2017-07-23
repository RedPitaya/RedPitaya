
%opengl software

%Red Pitaya Interpolation Demo

clc
close all
clear

path(path,'../')  

rp_ip='192.168.53.134';

%Generate a sine signal (make sure Output1 connected to Input1 and properly terminated)
generate(rp_ip,1,1,30.1e6,'sine');

%Acquire signal
sigs=acquire(rp_ip,16000,1,0);

fadc=125e6;

%Take channel 1
s1=sigs(:,1)';

%Prepare time vector
t1=((1:length(s1))-1)/fadc;

figure
plot(t1*1e9,s1,'x:')
xlabel('Time [ns]')
grid on


% Interpolation

N=length(s1);  % # acquired samples
M=N*9;         % # number of samples to be added by interpolation (interpolate 10x)

%Interpolate (sin(x)/x, fft implementation)
s2=fft_interp(s1,M);
%Update new sampling frequency
fs2=fadc*(M+N)/N; 

%Update time vector
t2=((1:length(s2))-1)/fs2;

hold on
plot(t2*1e9,s2,'r')
xlabel('Time [ns]')
grid on

legend('Raw samples','Interpolated signal')
title('Red Pitaya: 32 MHz signal acquisition (Interpolation algorithm)')
ylabel('ADC counts')

%Zoom 1000 ns
ax=axis;
ax(1)=100;
ax(2)=1000;
axis(ax);

