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

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <inttypes.h>

#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace std;

#include "xilinx-zynq.h"
#include <sys/types.h>

//xilinx_zynq::xilinx_zynq(sc_module_name name, const char *sk_descr,
//			Iremoteport_tlm_sync *sync)
//	: remoteport_tlm(name, -1, sk_descr, sync),
xilinx_zynq::xilinx_zynq(sc_module_name name, const char *sk_descr)
	: remoteport_tlm(name, -1, sk_descr),
	  rp_m_axi_gp0("rp_m_axi_gp0"),
	  rp_m_axi_gp1("rp_m_axi_gp1"),
	  rp_s_axi_gp0("rp_s_axi_gp0"),
	  rp_s_axi_gp1("rp_s_axi_gp1"),
	  rp_s_axi_hp0("rp_s_axi_hp0"),
	  rp_s_axi_hp1("rp_s_axi_hp1"),
	  rp_s_axi_hp2("rp_s_axi_hp2"),
	  rp_s_axi_hp3("rp_s_axi_hp3"),
	  rp_s_axi_acp("rp_s_axi_acp"),
	  rp_wires_in("wires_in", 20, 0),
	  rp_wires_out("wires_out", 0, 17),
	  rp_irq_out("irq_out", 0, 28),
	  pl2ps_irq("pl2ps_irq", 20),
	  ps2pl_irq("ps2pl_irq", 28),
	  ps2pl_rst("ps2pl_rst", 17)
{
	int i;

	m_axi_gp[0] = &rp_m_axi_gp0.sk;
	m_axi_gp[1] = &rp_m_axi_gp1.sk;

	s_axi_gp[0] = &rp_s_axi_gp0.sk;
	s_axi_gp[1] = &rp_s_axi_gp1.sk;

	s_axi_hp[0] = &rp_s_axi_hp0.sk;
	s_axi_hp[1] = &rp_s_axi_hp1.sk;
	s_axi_hp[2] = &rp_s_axi_hp2.sk;
	s_axi_hp[3] = &rp_s_axi_hp3.sk;
	s_axi_acp = &rp_s_axi_acp.sk;

	/* PL to PS Interrupt signals.  */
	for (i = 0; i < 20; i++) {
		rp_wires_in.wires_in[i](pl2ps_irq[i]);
	}

	/* PS to PL Interrupt signals.  */
	for (i = 0; i < 28; i++) {
		rp_irq_out.wires_out[i](ps2pl_irq[i]);
	}

	/* PS to PL resets.  */
	for (i = 0; i < 17; i++) {
		rp_wires_out.wires_out[i](ps2pl_rst[i]);
	}

	register_dev(0, &rp_s_axi_gp0);
	register_dev(1, &rp_s_axi_gp1);

	register_dev(2, &rp_s_axi_hp0);
	register_dev(3, &rp_s_axi_hp1);
	register_dev(4, &rp_s_axi_hp2);
	register_dev(5, &rp_s_axi_hp3);

	register_dev(6, &rp_s_axi_acp);

	register_dev(7, &rp_m_axi_gp0);
	register_dev(8, &rp_m_axi_gp1);
	register_dev(9, &rp_wires_in);
	register_dev(10, &rp_wires_out);
	register_dev(11, &rp_irq_out);
}
