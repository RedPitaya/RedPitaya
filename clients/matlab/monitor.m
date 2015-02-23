function back=monitor(ip_addr,loc,value)

command=['plink -l root -pw root ',ip_addr,' "/opt/bin/monitor ',loc,' ',value,'"'];
[c,data]=unix(command);


back=data;

end