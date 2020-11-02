/*
 * Xilinx SystemC/TLM-2.0 Zynq Wrapper.
 *
 * Written by Edgar E. Iglesias <edgar.iglesias@xilinx.com>
 *
 * Copyright (c) 2016, Xilinx Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "systemc.h"

#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"

#include "remote-port-tlm.h"
#include "remote-port-tlm-memory-master.h"
#include "remote-port-tlm-memory-slave.h"
#include "remote-port-tlm-wires.h"

class xilinx_zynq
: public remoteport_tlm
{
private:
	remoteport_tlm_memory_master rp_m_axi_gp0;
	remoteport_tlm_memory_master rp_m_axi_gp1;

	remoteport_tlm_memory_slave rp_s_axi_gp0;
	remoteport_tlm_memory_slave rp_s_axi_gp1;

	remoteport_tlm_memory_slave rp_s_axi_hp0;
	remoteport_tlm_memory_slave rp_s_axi_hp1;
	remoteport_tlm_memory_slave rp_s_axi_hp2;
	remoteport_tlm_memory_slave rp_s_axi_hp3;

	remoteport_tlm_memory_slave rp_s_axi_acp;

	remoteport_tlm_wires rp_wires_in;
	remoteport_tlm_wires rp_wires_out;
	remoteport_tlm_wires rp_irq_out;

public:
	/*
	 * M_AXI_GP 0 - 1.
	 * These sockets represent the High speed PS to PL interfaces.
	 * These are AXI Slave ports on the PS side and AXI Master ports
	 * on the PL side.
	 *
	 * Used to transfer data from the PS to the PL.
	 */
	tlm_utils::simple_initiator_socket<remoteport_tlm_memory_master> *m_axi_gp[2];

	/*
	 * S_AXI_GP0 - 1.
	 * These sockets represent the High speed IO Coherent PL to PS
	 * interfaces.
	 *
	 * HP0 - 3.
	 * These sockets represent the High performance dataflow PL to PS interfaces.
	 *
	 * ACP
	 * Accelerator Coherency Port, used to transfered coherent data to
	 * the PS via the Cortex-A9 subsystem.
	 *
	 * These are AXI Master ports on the PS side and AXI Slave ports
	 * on the PL side.
	 *
	 * Used to transfer data from the PL to the PS.
	 */
	tlm_utils::simple_target_socket<remoteport_tlm_memory_slave> *s_axi_gp[2];
	tlm_utils::simple_target_socket<remoteport_tlm_memory_slave> *s_axi_hp[4];
	tlm_utils::simple_target_socket<remoteport_tlm_memory_slave> *s_axi_acp;

	/* PL (fabric) to PS interrupt signals.  */
	sc_vector<sc_signal<bool> > pl2ps_irq;

	/* PS to PL Interrupt signals.  */
	sc_vector<sc_signal<bool> > ps2pl_irq;

	/* FPGA out resets.  */
	sc_vector<sc_signal<bool> > ps2pl_rst;

	xilinx_zynq(sc_core::sc_module_name name, const char *sk_descr);
	//xilinx_zynq(sc_core::sc_module_name name, const char *sk_descr,
	//		Iremoteport_tlm_sync *sync = NULL);
};
