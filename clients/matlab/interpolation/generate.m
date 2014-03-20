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

%Calling generate utility on Red Pitaya
command=['plink -l root -pw root ',ip_addr,' "/opt/bin/generate ',num2str(chan),' ',num2str(amp),' ',num2str(freq),' ',type,'"'];
[c,data]=unix(command);

void=0;

end
