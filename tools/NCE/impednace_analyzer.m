%% Define Red Pitaya as TCP/IP object
clear all
close all
clc
IP = '192.168.178.102';                % Input IP of your Red Pitaya...
port = 5000;                           % If you are using WiFi then IP is:                  
tcpipObj = tcpip(IP, port);            % 192.168.128.1
tcpipObj.InputBufferSize = 16384*32;

%% Open connection with your Red Pitaya

fopen(tcpipObj);
tcpipObj.Terminator = 'CR/LF';

flushinput(tcpipObj);
flushoutput(tcpipObj);


dec_vec=[1,8,64,1024,8192,65536]; 
Rs_vec=[30,75,300,750,3e3,7.5e3,30e3,75000,430e3,3e6];
i2c_vec={'0xfffe','0xfffd','0xfffb','0xfff7','0xffef','0xffdf','0xffbf','0xff7f','0xfeff','0xfdff','0x0000'};
Measurments=[];
Measurments_working=[];
f_vec=[]; 

fprintf(tcpipObj,'ACQ:RST');
fprintf(tcpipObj,'GEN:RST');
%fprintf(tcpipObj,'SOUR1:FUNC SINE');       
%fprintf(tcpipObj,'SOUR1:VOLT 0.2'); 


fprintf(tcpipObj,'SOUR2:FUNC SINE');       
fprintf(tcpipObj,'SOUR2:VOLT 0.2')
fprintf(tcpipObj,'SOUR2:VOLT:OFFS 0.3')

fprintf(tcpipObj,'ACQ:TRIG:LEV 0.6');
fprintf(tcpipObj,'ACQ:TRIG:DLY 8192');

% Wait for data writing to Red Pitaya
pause(0.1) 
 
%INIT

% Vector of frequencies for measurement
scale=1 ; % 0 - LINEAR
start_frequency=10;
end_frequency=1000000;
%STEPS > 1!!!
steps=100;

frequency_step = ( end_frequency - start_frequency ) / ( steps - 1 );
a = log10(start_frequency);
b = log10(end_frequency); 
c = (b - a)/(steps - 1);



for step=0:1:steps-1
    
if scale==0
    f_0=start_frequency+(frequency_step*step);
    f_vec=[f_vec round(f_0)]; 
else
    f_0=10^(c*step+a);
    f_vec=[f_vec round(f_0)];
end
end

j=1;
k=1;

while (j<=steps)
   
   
R_shunt=Rs_vec(k);
i2c=i2c_vec{k};
%command=['sshpass -p root ssh root@',IP,' "/opt/bin/i2c ',i2c,'"'];
%unix(command);
command=['sshpass -p root ssh root@',IP,' "LD_LIBRARY_PATH=/opt/redpitaya/lib /opt/redpitaya/bin/i2c ',i2c,'"'];
unix(command);
% Wait for data writing to Red Pitaya
pause(0.1)

f_sweep=f_vec(j)

if f_sweep>=60000 
   i=1; 
   elseif f_sweep>=8000
   i=2;
   elseif f_sweep>=800
   i=3;
   elseif f_sweep>=80
   i=4;
   elseif f_sweep>=8
   i=5;
   elseif f_sweep>=1
   i=6;
end

%i=1
dec = num2str(dec_vec(i))

f=num2str(f_sweep);

fprintf(tcpipObj,['SOUR1:FREQ:FIX ' f]);
fprintf(tcpipObj,'OUTPUT1:STATE ON');      


fprintf(tcpipObj,['SOUR2:FREQ:FIX ' f]);
fprintf(tcpipObj,'OUTPUT2:STATE ON'); 
% Set decimation vale (sampling rate) in respect to you 
% acquired signal frequency

fprintf(tcpipObj, ['ACQ:DEC ' dec]);

% Wait for data writing to Red Pitaya
pause(0.1)                                          

%% Start & Trigg
% Trigger source setting must be after ACQ:START
% Set trigger to source 1 positive edge

fprintf(tcpipObj,'ACQ:START');

pause(20*(1/f_sweep));
fprintf(tcpipObj,'ACQ:TRIG CH1_PE');   % Must be after ACQ:START

% Wait for trigger
% Until trigger is true wait with acquiring
% Be aware of while loop if trigger is not achieved
% Ctrl+C will stop code executing in Matlab  


while 1
     trig_rsp=query(tcpipObj,'ACQ:TRIG:STAT?');
   
     if strcmp('TD',trig_rsp(1:2))                          % Read TD only 
     break
   
     end
 end
 
 
% Read data from buffer 
min_periodes=10;
N =round((min_periodes*125e6)/(f_sweep*dec_vec(i))); 
if N>16384;
N=16384;
end
size=num2str(N);
signal_str=query(tcpipObj,['ACQ:SOUR1:DATA:OLD:N? ' size]); 
signal_str_2=query(tcpipObj,['ACQ:SOUR2:DATA:OLD:N? ' size]); 

%signal_str=query(tcpipObj,['ACQ:SOUR1:DATA?']); 
%signal_str_2=query(tcpipObj,['ACQ:SOUR2:DATA?']); 

signal_num=str2num(signal_str(1,2:length(signal_str)-3));
signal_num_2=str2num(signal_str_2(1,2:length(signal_str_2)-3));

signal_num=signal_num-mean(signal_num);
signal_num_2=signal_num_2-mean(signal_num_2);
%signal_num=signal_num(1:N);
%signal_num_2=signal_num_2(1:N);

U_dut = signal_num-signal_num_2;
I_dut = signal_num_2./R_shunt;


buffer_ln=N;
time_delay=8200;
trigger_level_1=0;
trigger_level=ones(1,buffer_ln).*trigger_level_1;
time_delay=-time_delay+8192;

plot(signal_num,'*')
hold on
plot(signal_num_2,'xr')
hold on
plot(trigger_level,'r')
hold on
plot((time_delay.*ones(1,buffer_ln)),signal_num, 'k' )
grid on
hold off

T=(1/125e6)*dec_vec(i);                       % Sampling time [seconds] 
dT=(0:1:N-1)*T;  
t=0:1:N-1;                                    % time increment
w_out=f_sweep*2*pi;

%% Set empty vectors for two lock-in components (X,Y (sin,cos)) for sampled input signals 

U_dut_sampled_X=zeros(1,N);
U_dut_sampled_Y=zeros(1,N);
I_dut_sampled_X=zeros(1,N);
I_dut_sampled_Y=zeros(1,N);

%% Accurie  signals  U_in_1 and U_in_2 from RedPitaya for N-sampels time and Calculate for Lock in and save in vectors for sampels

U_dut_sampled_X=U_dut.*sin(t*T*w_out);
U_dut_sampled_Y=U_dut.*sin(t*T*w_out-pi/2);

I_dut_sampled_X=I_dut.*sin(t*T*w_out);
I_dut_sampled_Y=I_dut.*sin(t*T*w_out-pi/2);


%% Calculate two components of lock-in for both mesured voltage signals

X_component_lock_in_1=trapz(U_dut_sampled_X,dT);
Y_component_lock_in_1=trapz(U_dut_sampled_Y,dT);

X_component_lock_in_2=trapz(I_dut_sampled_X,dT);
Y_component_lock_in_2=trapz(I_dut_sampled_Y,dT);

U_dut_amp=(sqrt((X_component_lock_in_1)^2+(Y_component_lock_in_1)^2))*2;
Phase_U_dut=atan2(Y_component_lock_in_1,X_component_lock_in_1);

%% Calculate amplitude and angle of I_dut

I_dut_amp=(sqrt((X_component_lock_in_2)^2+(Y_component_lock_in_2)^2))*2;
Phase_I_dut=atan2(Y_component_lock_in_2,X_component_lock_in_2);

%% Calculate amplitude of current trough impedance and amplitude of impedance

Z_amp=(U_dut_amp/I_dut_amp);            % Amaplitude of impednace

Measurments_working=[Measurments_working ;[j,Z_amp,R_shunt]];

Phase_Z_rad=(Phase_U_dut-Phase_I_dut);
Phase_check=(Phase_U_dut-Phase_I_dut)*(180/pi);
if Phase_check<=-180
   Phase_Z=(Phase_U_dut-Phase_I_dut)*(180/pi)+360;
   elseif Phase_check>=180
   Phase_Z=(Phase_U_dut-Phase_I_dut)*(180/pi)-360;
else 
   Phase_Z=Phase_check;
end

if (Z_amp <= 10 && k==1) | (Z_amp >= 1e6 && k==10)

save_at=j;
Measurments=[Measurments ;[j,f_sweep,Z_amp,R_shunt, Phase_Z]];
j=j+1;

else
    
if Z_amp >=3*R_shunt | Z_amp <= 1/3*R_shunt   

    if Z_amp > 500e3
    k=10;
    elseif Z_amp > 100e3 && Z_amp <= 500e3
    k=9;
    elseif Z_amp > 50e3 && Z_amp <= 100e3
    k=8;
    elseif Z_amp > 10e3 && Z_amp <= 50e3
    k=7;
    elseif Z_amp > 5e3 && Z_amp <= 10e3
    k=6;
    elseif Z_amp > 1000 && Z_amp <= 5e3
    k=5;
    elseif Z_amp > 500 && Z_amp <= 1000
    k=4;
    elseif Z_amp > 100 && Z_amp <= 500
    k=3;
    elseif Z_amp > 50 && Z_amp <= 100
    k=2;
    elseif Z_amp <=50
    k=1;
    end
    
    j=j-1;
   
    if j==0;
       j=1;
    elseif save_at==j
       j=j+1;
    end
     
     
else
 
save_at=j;
Measurments=[Measurments ;[j,f_sweep,Z_amp,R_shunt, Phase_Z]];
j=j+1;

end

end
end


figure
subplot(2,1,1); 
if scale==0
plot(Measurments(:,2), Measurments(:,3),'-ob',Measurments(:,2),Measurments(:,4),'-or')
else
semilogx(Measurments(:,2), Measurments(:,3),'-ob',Measurments(:,2),Measurments(:,4),'-or')
end
grid on
subplot(2,1,2);
if scale==0
plot(Measurments(:,2), Measurments(:,5))
else
semilogx(Measurments(:,2), Measurments(:,5))
end
grid on

% figure
% if scale==0
% plot(Measurments_working(:,1), Measurments_working(:,2),'-ob',Measurments_working(:,1),Measurments_working(:,3),'-or')
% else
% semilogx(Measurments_working(:,1), Measurments_working(:,2),'-ob',Measurments_working(:,1),Measurments_working(:,3),'-or')
% end
% grid on


%fprintf(tcpipObj,'OUTPUT1:STATE OFF');  

%fprintf(tcpipObj,'OUTPUT2:STATE OFF'); % Set output to ON 
fclose(tcpipObj)


