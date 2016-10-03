==============
Identification
==============

-----------
Description
-----------

------------
Register set
------------

+---------+----------------+
| address | register name  |
+=========+================+
| 0x00    | ID constant    |
+---------+----------------+
| 0x04    | DNA [32-1: 0]  |
+---------+----------------+
| 0x08    | DNA [57-1:32]  |
+---------+----------------+
| 0x0c    | reserved       |
+---------+----------------+
| 0x10    | efuse          |
+---------+----------------+
| 0x14    | reserved       |
+---------+----------------+
| 0x18    | reserved       |
+---------+----------------+
| 0x1c    | reserved       |
+---------+----------------+
| 0x20    | GITH[32*0+:32] |
+---------+----------------+
| 0x24    | GITH[32*1+:32] |
+---------+----------------+
| 0x28    | GITH[32*2+:32] |
+---------+----------------+
| 0x2c    | GITH[32*3+:32] |
+---------+----------------+
| 0x3c    | GITH[32*4+:32] |
+---------+----------------+


+---------+----------+---------+
| bit     | 31:4     | 3:0     |
+=========+==========+=========+
| name    | rsverved | release |
+---------+----------+---------+
| R/W     | RO       | RO      |
+---------+----------+---------+
| reset   | 0x0      | 0x1     |
+---------+----------+---------+



   always_ff @(posedge bus.clk)
   if (!bus.rstn) begin
     bus.ack <= 1'b0;
   end else begin
     bus.ack <= sys_en;
     casez (bus.addr[BDW-1:0])
       // ID
       'h00:  bus.rdata <= ID;
   // TODO: this is compatible with older releases, but not properly 64bit alligned
       'h04:  bus.rdata <= {                            dna_value[32-1: 0]};
       'h08:  bus.rdata <= {~dna_done, {64-57-1{1'b0}}, dna_value[57-1:32]};
       'h10:  bus.rdata <= efuse;
   //    // EFUSE
   //    'h08:  bus.rdata <= efuse;
   //    // DNA
   //    'h10:  bus.rdata <= {                            dna_value[32-1: 0]};
   //    'h14:  bus.rdata <= {~dna_done, {64-57-1{1'b0}}, dna_value[57-1:32]};
       // GITH
       'h20:  bus.rdata <= GITH[32*0+:32];
       'h24:  bus.rdata <= GITH[32*1+:32];
       'h28:  bus.rdata <= GITH[32*2+:32];
       'h2c:  bus.rdata <= GITH[32*3+:32];
       'h3c:  bus.rdata <= GITH[32*4+:32];
       default: bus.rdata <= 'x;
     endcase
   end
