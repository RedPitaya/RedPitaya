// (c) Copyright(C) 2013 - 2018 by Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.

#ifndef _B_TRANSPORT_CONVERTER_H_
#define _B_TRANSPORT_CONVERTER_H_

#include <systemc>
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include <utility>
#include <vector>
#include "xtlm.h"
#include <queue>

template<int IN_WIDTH, int OUT_WIDTH>
class b_transport_converter: public sc_core::sc_module 
{
    enum TLM_IF_TYPE
    {
        B_TRANSPORT = 0,
        NB_TRANSPORT,
        TRANSPORT_DBG,
        DMI_IF,
        INVALID_IF
    };
    typedef std::vector<std::pair<sc_dt::uint64, sc_dt::uint64>> addr_range_list;

    public:
        SC_HAS_PROCESS(b_transport_converter);
        b_transport_converter<IN_WIDTH, OUT_WIDTH>(sc_core::sc_module_name name): 
            sc_module(name)
    {
        target_socket.register_b_transport(
                this, &b_transport_converter<IN_WIDTH, OUT_WIDTH>::b_transport);
        initiator_socket.register_nb_transport_bw(
                this, &b_transport_converter<IN_WIDTH, OUT_WIDTH>::nb_transport_bw);

    }

        //simple tlm target/initiator socket...
        tlm_utils::simple_target_socket<b_transport_converter<IN_WIDTH, OUT_WIDTH>, IN_WIDTH>    target_socket;
        tlm_utils::simple_initiator_socket<b_transport_converter<IN_WIDTH, OUT_WIDTH>, OUT_WIDTH> initiator_socket;


    public:
        void b_transport(tlm::tlm_generic_payload& payload, sc_core::sc_time& time)
        {
            tlm::tlm_phase phase = tlm::BEGIN_REQ; //for nb_transport_fw
            switch(get_tlm_if_type(payload.get_address()))
            {
                case B_TRANSPORT:
                    initiator_socket->b_transport(payload, time);
                    break;

                case NB_TRANSPORT:
                    initiator_socket->nb_transport_fw(payload, phase, time);
                    wait(resp_complete_event); //! Wait for the response to complete
                    break;

                case TRANSPORT_DBG:
                    initiator_socket->transport_dbg(payload);
                    break;

                case DMI_IF:
                    break;

                default:
                    SC_REPORT_ERROR(this->name(), "Address not mapped to any of the TLM IF type");
            }
        }

        tlm::tlm_sync_enum
            nb_transport_bw(tlm::tlm_generic_payload& payload, 
                    tlm::tlm_phase& phase, sc_core::sc_time& time)
            {
                if(phase == tlm::BEGIN_RESP) {
                    resp_complete_event.notify();
                    phase = tlm::END_RESP;
                    return tlm::TLM_UPDATED;
                }
                return tlm::TLM_ACCEPTED;
            }

    private:
        TLM_IF_TYPE get_tlm_if_type(unsigned long long address)
        {
            //check for b_transport addresses
            for(auto& addr_range: m_b_transport_addr_list) {
                if(address >= addr_range.first && address < addr_range.second) {
                    return B_TRANSPORT;
                }
            }

            //check for nb_transport addresses
            for(auto& addr_range: m_nb_transport_addr_list) {
                if(address >= addr_range.first && address < addr_range.second) {
                    return NB_TRANSPORT;
                }
            }
            //check for dbg_transport addresses
            for(auto& addr_range: m_dbg_transport_addr_list) {
                if(address >= addr_range.first && address < addr_range.second) {
                    return TRANSPORT_DBG;
                }
            }

            //By default return NB_TRANSPORT
            return NB_TRANSPORT;
        }

        //Start and End Address List for each of interfaces...
        static addr_range_list  m_b_transport_addr_list;
        static addr_range_list  m_nb_transport_addr_list;
        static addr_range_list  m_dbg_transport_addr_list;

        //event to notify completion of transaction
        sc_core::sc_event  resp_complete_event;
};

template<int IN_WIDTH, int OUT_WIDTH>
typename b_transport_converter<IN_WIDTH,OUT_WIDTH>::addr_range_list b_transport_converter<IN_WIDTH,OUT_WIDTH>::m_b_transport_addr_list = {std::make_pair(0, 0)};
template<int IN_WIDTH, int OUT_WIDTH>
typename b_transport_converter<IN_WIDTH,OUT_WIDTH>::addr_range_list b_transport_converter<IN_WIDTH,OUT_WIDTH>::m_nb_transport_addr_list = {std::make_pair(0, 0)};
template<int IN_WIDTH, int OUT_WIDTH>
typename b_transport_converter<IN_WIDTH,OUT_WIDTH>::addr_range_list b_transport_converter<IN_WIDTH,OUT_WIDTH>::m_dbg_transport_addr_list = {std::make_pair(0, 0)};

using namespace xtlm;
namespace zynq_tlm {
class xtlm_aximm_targ_rd_b_util: public xtlm_aximm_target_rd_socket_util
{
public:
	xtlm_aximm_targ_rd_b_util(sc_core::sc_module_name p_name,
			aximm::granularity g_hint, int width,
			xtlm_aximm_initiator_rd_socket_util* init_util) :
			xtlm_aximm_target_rd_socket_util(p_name, g_hint, width), m_init_util(
					init_util)
	{

	}
	void b_transport_cb(xtlm::aximm_payload& trans, sc_core::sc_time& delay)
	{
		m_init_util->b_transport(trans, delay);
	}

	unsigned int transport_dbg_cb(xtlm::aximm_payload& trans)
	{
		return m_init_util->transport_dbg(trans);
	}
private:
	xtlm::xtlm_aximm_initiator_rd_socket_util* m_init_util;
};

class xtlm_aximm_targ_wr_b_util: public xtlm_aximm_target_wr_socket_util
{
public:
	xtlm_aximm_targ_wr_b_util(sc_core::sc_module_name p_name,
			aximm::granularity g_hint, int width,
			xtlm_aximm_initiator_wr_socket_util* init_util) :
			xtlm_aximm_target_wr_socket_util(p_name, g_hint, width), m_init_util(
					init_util)
	{

	}
	void b_transport_cb(xtlm::aximm_payload& trans, sc_core::sc_time& delay)
	{
		m_init_util->b_transport(trans, delay);
	}
	unsigned int transport_dbg_cb(xtlm::aximm_payload& trans)
	{
		return m_init_util->transport_dbg(trans);
	}
private:
	xtlm::xtlm_aximm_initiator_wr_socket_util* m_init_util;
};


class xsc_xtlm_aximm_tran_buffer: public sc_module
{
public:
	xtlm::xtlm_aximm_target_socket *in_rd_socket;
	xtlm::xtlm_aximm_target_socket *in_wr_socket;
	xtlm::xtlm_aximm_initiator_socket *out_rd_socket;
	xtlm::xtlm_aximm_initiator_socket *out_wr_socket;

	SC_HAS_PROCESS(xsc_xtlm_aximm_tran_buffer);
	xsc_xtlm_aximm_tran_buffer(const sc_core::sc_module_name &name) :
			sc_module(name)
	{
		in_rd_socket = new xtlm::xtlm_aximm_target_socket("in_rd_socket", 0);
		in_wr_socket = new xtlm::xtlm_aximm_target_socket("in_wr_socket", 0);
		out_rd_socket = new xtlm::xtlm_aximm_initiator_socket("out_rd_socket",
				0);
		out_wr_socket = new xtlm::xtlm_aximm_initiator_socket("out_wr_socket",
				0);

		out_rd_socket_util = new xtlm::xtlm_aximm_initiator_rd_socket_util(
				"out_rd_socket_util", xtlm::aximm::TRANSACTION, 0);
		out_wr_socket_util = new xtlm::xtlm_aximm_initiator_wr_socket_util(
				"out_wr_socket_util", xtlm::aximm::TRANSACTION, 0);

		in_rd_socket_util = new xtlm_aximm_targ_rd_b_util(
				"in_rd_socket_util", xtlm::aximm::TRANSACTION, 0,
				out_rd_socket_util);
		in_wr_socket_util = new xtlm_aximm_targ_wr_b_util(
				"in_wr_socket_util", xtlm::aximm::TRANSACTION, 0,
				out_wr_socket_util);
		in_rd_socket->bind(in_rd_socket_util->rd_socket);
		in_wr_socket->bind(in_wr_socket_util->wr_socket);

		out_rd_socket_util->rd_socket.bind(*out_rd_socket);
		out_wr_socket_util->wr_socket.bind(*out_wr_socket);

		SC_METHOD(read_request_process);
		sensitive << in_rd_socket_util->transaction_available;
		sensitive << out_rd_socket_util->transaction_sampled;
		dont_initialize();

		SC_METHOD(read_response_process);
		sensitive << out_rd_socket_util->data_available;
		sensitive << in_rd_socket_util->data_sampled;
		dont_initialize();

		SC_METHOD(write_request_process);
		sensitive << in_wr_socket_util->transaction_available;
		sensitive << out_wr_socket_util->transaction_sampled;
		dont_initialize();

		SC_METHOD(write_response_process);
		sensitive << in_wr_socket_util->resp_sampled;
		sensitive << out_wr_socket_util->resp_available;
		dont_initialize();
	}
	~xsc_xtlm_aximm_tran_buffer()
	{
		delete in_rd_socket;
		delete in_wr_socket;
		delete out_rd_socket;
		delete out_wr_socket;

		delete out_rd_socket_util;
		delete out_wr_socket_util;
		delete in_rd_socket_util;
		delete in_wr_socket_util;
	}
private:
	xtlm_aximm_targ_rd_b_util* in_rd_socket_util;
	xtlm_aximm_targ_wr_b_util* in_wr_socket_util;
	xtlm::xtlm_aximm_initiator_rd_socket_util* out_rd_socket_util;
	xtlm::xtlm_aximm_initiator_wr_socket_util* out_wr_socket_util;
	std::queue<xtlm::aximm_payload*> rd_request_queue;
	std::queue<xtlm::aximm_payload*> wr_request_queue;
	std::queue<xtlm::aximm_payload*> rd_response_queue;
	std::queue<xtlm::aximm_payload*> wr_response_queue;

	void read_response_process()
	{
		if (out_rd_socket_util->is_data_available())
		{
			rd_response_queue.push(out_rd_socket_util->get_data());
		}

		if (in_rd_socket_util->is_master_ready()
				&& rd_response_queue.size() > 0)
		{
			sc_core::sc_time zero_time = SC_ZERO_TIME;
			in_rd_socket_util->send_data(*rd_response_queue.front(), zero_time);
			rd_response_queue.pop();
		}
	}
	void write_response_process()
	{
		if (out_wr_socket_util->is_resp_available())
			wr_response_queue.push(out_wr_socket_util->get_resp());

		if (in_wr_socket_util->is_master_ready() && wr_response_queue.size())
		{
			sc_core::sc_time zero_time = SC_ZERO_TIME;
			in_wr_socket_util->send_resp(*wr_response_queue.front(), zero_time);
			wr_response_queue.pop();
		}
	}
	void read_request_process()
	{
		if (in_rd_socket_util->is_trans_available())
			rd_request_queue.push(in_rd_socket_util->get_transaction());

		if (out_rd_socket_util->is_slave_ready() && rd_request_queue.size() > 0)
		{
			sc_core::sc_time zero_time = SC_ZERO_TIME;
			out_rd_socket_util->send_transaction(*rd_request_queue.front(),
					zero_time);
			rd_request_queue.pop();
		}
	}

	void write_request_process()
	{
		if (in_wr_socket_util->is_trans_available())
			wr_request_queue.push(in_wr_socket_util->get_transaction());

		if (out_wr_socket_util->is_slave_ready() && wr_request_queue.size() > 0)
		{
			sc_core::sc_time zero_time = SC_ZERO_TIME;
			out_wr_socket_util->send_transaction(*wr_request_queue.front(),
					zero_time);
			wr_request_queue.pop();
		}
	}
};
}


#endif /* _B_TRANSPORT_CONVERTER_H_ */

