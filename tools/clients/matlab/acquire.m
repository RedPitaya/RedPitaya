function data=acquire(ip_addr,samps,dec,ave_dec)

% Red Pitaya Acquire Utility
% Borut Baricevic
% function data=f_acquire(ip_addr,samps,dec)
% 
% Returns two columns containing the acquired samples on two channels
%
% ip_addr: Red Pitaya IP address
% samp   : Acquired samples (max 16k)
% dec    : Red Pitaya decimation (supported: 1, 8, 64, 1024, 8192,65536)  
% ave_dec: Enables averaging at decimation (0 disabled, 1 enabled)

% set up commands uppon running O.S.
% added by David Zuliani 2014/01/23
%  for MACOSX and LINUX O.S. you need to set up the ssh authorized key
%  file  inside a .ssh directory of the RedPitaya O.S. Unfortunately at
%  every RP reboot you must create again the .ssh directory and the
%  authorized key file
switch computer
    case {'PCWIN','PCWIN64'}
        CMD='plink -l root -pw root ';
        CMDC='pscp -pw root root@';
        CMDD='del';
    case {'GLNXA64','MACI64'}
        CMD='ssh -l root ';
        CMDC='scp root@';
        CMDD='rm';
    otherwise
        disp('UNKNOW O.S.');
        return
end

%Set averaging FPGA register
command=[CMD,ip_addr,' "/opt/bin/monitor 0x40100028 0x',num2str(ave_dec),'"'];
[c,data]=unix(command);

%Acquire data to Red Pitaya memory
command=[CMD,ip_addr,' "/opt/bin/acquire ',num2str(samps),' ',num2str(dec),' > /tmp/acq_tmp.txt"'];
[c,data]=unix(command);

here=pwd;
%Copy file to current Matlab directory
command2=[CMDC,ip_addr,':/tmp/acq_tmp.txt  ',here];
[c,data]=unix(command2);

%Access data
%data=importdata('acq_tmp.txt');

data=load('acq_tmp.txt');


%Erase file
unix([CMDD,' acq_tmp.txt']);

end