

opengl software

clc
close all
clear

fadc=125e6;


% Generating TESTBENCH signal

% From real ADC acquisition
aa=importdata('acq10kA.txt');  % Rectangular signal 

% Adding some testcases
ss=aa(:,1);
ss=[ss; zeros(100000,1)];
xx=(1:length(ss))'/fadc;


ss(12500:end)=ss(12500:end)*0+(xx(12500:end)>(12750/fadc)).*8192.*cos(2*pi*10e6*xx(12500:end));
ss(42500:end)=(xx(42500:end)>(42750/fadc)).*8192.*cos(2*pi*2e3*xx(42500:end));

% Rounding to integers
ss=round(ss);


figure
plot(ss)
grid on


% High level SW coefficient calculation

% Input parameters
% Zero-Pole ratio constant
 zop=0.9539
% Pole angular frequency 
 p=1.5904e5
%Overshoot filtering parameter
alp2=0.15;  
 
% SW Calculates zero angular frequency
 z=zop*p;
 
 
% SW calculates Z-domain coefficents from L-domin pole and zero 
               % Explanation:  
alp=1-z/fadc;  % z=exp(sT)
bet=1-p/fadc;  % exp(x) =approx= 1+x+....
               % (z-bet)/(z-alp) =approx= (1+sT-bet)/(1+sT-alp)
               % (z-bet)/(z-alp) * (s+z)/(s+p) = 1

omalp=1-alp;   %Complementar values
ombet=1-bet;

%%%%%
% Neutral values (TO BE IMPLEMENTED AS FPGA DEFAULTS !!!)
% ombet=0;
% omalp=0;
% alp2=0;
%
%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Quantized model: bit shift definitions

shl=18;   
shr1=10;  
shr2=10;  

shr3=8;   


% SW CALCULATES FPGA PARAMS

BB=round(ombet.*2^(shr1+shr2+shr3))

shrp=25;  
AA=round(omalp.*2^(shrp))

shrp2=16; 
PP=round(alp2.*2^(shrp2))

shrk=24;  
KK=round((1-alp2).*2^(shrk))


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% FPGA SIMULATION: INITIALIZING REGISTERS AS ARRAYS

rg01=ss*0;
rg02=ss*0;
rg1=ss*0;
rg2=ss*0;
rg3=ss*0;

rg4=ss*0;
rg5=ss*0;


% The simulation

for jxx=2:length(ss)
    

   
   rg01(jxx)=ss(jxx-1)*2^(shl);
   rg02(jxx)=floor(ss(jxx-1)*BB*2^(-shr1));
   rg1(jxx)=rg02(jxx-1)-rg01(jxx-1);     
   rg2(jxx)=floor((rg01(jxx-1)+rg1(jxx-1))*2^(-shr2));  
   
   rg3(jxx)=rg2(jxx-1)+rg3(jxx-1)-floor(AA*rg3(jxx-1)*2^(-shrp));
  
   rg4(jxx)=floor(2^(-shr3)*rg3(jxx-1))+floor(PP*rg4(jxx-1)*2^(-shrp2));
  
   rg5(jxx)=floor(rg4(jxx-1)*KK*2^(-shrk));
   
   % SAT 14 bits (signed)
   
   rg5(jxx)=max(rg5(jxx),-2^13);
   rg5(jxx)=min(rg5(jxx),(2^13)-1);
   
  
end

% Timevector for plot
ttt=((1:length(rg5))-1)/fadc;

figure
plot(ttt*1e6,ss)
grid on
hold on
plot(ttt*1e6,rg5,'r')
legend('Input','Output')





