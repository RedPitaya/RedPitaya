function back=monitor(ip_addr,loc,value)

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

command=[CMD,' -l root -pw root ',ip_addr,' "/opt/bin/monitor ',loc,' ',value,'"'];
[c,data]=unix(command);


back=data;

end