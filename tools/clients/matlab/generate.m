function void=generate(ip_addr,chan,amp,freq,type)

% Red Pitaya Generate Utility
% Borut Baricevic
% function data=f_generate(ip_addr,chan,amp,freq,type)
% 
% Returns two columns containing the acquired samples on two channels
%
% ip_addr: Red Pitaya IP address
% chan   : Channel to generate signal on [1, 2].
% amp    : Peak-to-peak signal amplitude in Vpp [0.0 - 2.0].
% freq   : Signal frequency in Hz [0.0 - 6.2e+07].
% type:  : Signal type [sine, sqr, tri]

% set up commands uppon running O.S.
% added by David Zuliani 2014/01/23
%  for MACOSX and LINUX O.S. you need to set up the ssh authorized key
%  file  inside a .ssh directory of the RedPitaya O.S. Unfortunately at
%  every RP reboot you must create again the .ssh directory and the
%  authorized key file
switch computer
    case {'PCWIN','PCWIN64'}
        CMD='plink -l root -pw root ';
    case {'GLNXA64','MACI64'}
        CMD='ssh -l root ';
    otherwise
        disp('UNKNOW O.S.');
        return
end

%Calling generate utility on Red Pitaya
command=[CMD,ip_addr,' "/opt/bin/generate ',num2str(chan),' ',num2str(amp),' ',num2str(freq),' ',type,'"'];

[c,data]=unix(command);

void=0;

end
