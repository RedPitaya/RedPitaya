#/bin/bash
sigrok-cli -i ./*.sr -P spi:mosi=MOSI:clk=CLK:cs=CS# | grep "^.\{9\}$" > sigrock.out
