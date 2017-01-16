##################
AXI4-Lite protocol
##################

The **AXI4-Lite** protocol is part of **AMBA4** protocols defined by **ARM**.
The specification is open and broadly adopted.
 
This document is not intended as a full ddescription of the protocol,
instead it focuses on the implementation used in Red Pitaya RTL modules.

***************
Protocol design
***************

The interface is split into 5 data streams:

* write address
* write data
* write response
* read address
* read data and response

Each stream has its own handshaking signals, so they can handled separately.
For example to solve timing issues each of the streams can register
forward signals or the backpressure signal separately from the other streams.
This separation also allows for interconnect components (arbiters, address decoders)
to be optimized depending on trafic patterns on each of the streams.


+-------------------+----------------+
|  signal           | description    |
+===================+================+
| ``ACLK``          |                |
+-------------------+----------------+
| ``ARESETn``       |                |
+-------------------+----------------+
| ``AWADDR``        |                |
+-------------------+----------------+
| ``AWPROT``        |                |
+-------------------+----------------+
| ``AWVALID``       |                |
+-------------------+----------------+
| ``AWRERADY``      |                |
+-------------------+----------------+

.. wavedrom::

   {signal: [
     ['system',
       {name: 'ACLK'   , wave: 'p.....|...'},
       {name: 'ARESETn', wave: 'x.345x|=.x', data: ['head', 'body', 'tail', 'data']},
     ],
     {},
     ['write address',
       {name: 'AWADDR' , wave: '0.1..0|1.0'},
       {name: 'AWPROT' , wave: '0.1..0|1.0'},
       {name: 'AWVALID', wave: '1.....|01.'},
       {name: 'AWREADY', wave: '1.....|01.'},
     ],
     {},
     ['write data',
       {name: 'WDATA'  , wave: '1.....|01.'},
       {name: 'WSTRB'  , wave: '1.....|01.'},
       {name: 'WVALID' , wave: '1.....|01.'},
       {name: 'WREADY' , wave: '1.....|01.'},
     ],
     {},
     ['write response',
       {name: 'BRESP'  , wave: '1.....|01.'},
       {name: 'BVALID' , wave: '1.....|01.'},
       {name: 'BREADY' , wave: '1.....|01.'},
     ],
     {},
     ['read address',
       {name: 'ARADDR' , wave: '0.1..0|1.0'},
       {name: 'ARPROT' , wave: '0.1..0|1.0'},
       {name: 'ARVALID', wave: '1.....|01.'},
       {name: 'ARREADY', wave: '1.....|01.'},
     ],
     {},
     ['read data',
       {name: 'RDATA'  , wave: '1.....|01.'},
       {name: 'RRESP'  , wave: '1.....|01.'},
       {name: 'RVALID' , wave: '1.....|01.'},
       {name: 'RREADY' , wave: '1.....|01.'}
     ]
   ]}
