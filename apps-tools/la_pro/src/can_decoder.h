#pragma once

#include "decoder.h"

#ifndef CLI
#include "DataManager.h"
#include "CustomParameters.h"
#include <pthread.h>
#endif

#include <cstring>
#include <string>
#include <vector>
#include <deque>

enum FrameFormat { None, Standart, Extended };
struct CANParameters
{
	uint8_t  can_rx; // 0...7
	uint32_t nominal_bitrate;  // default value  1000000 (bits/s)
	uint32_t fast_bitrate; // default value  2000000 (bits/s)
	float    sample_point; // default value  70.0 (%)
	uint16_t frame_limit;
	uint32_t acq_speed;
	uint8_t  invert_bit; 
};

#ifndef CLI
//custom CCANParameter
class CCANParameter : public CDecoderParameter<CANParameters>
{
public:
	CCANParameter(std::string _name, CBaseParameter::AccessMode _access_mode, CANParameters _value, int _fpga_update)
		:CDecoderParameter(_name, _access_mode, _value, _fpga_update, 0, 0){};

	JSONNode GetJSONObject()
	{
		JSONNode n(JSON_NODE);
		n.set_name(this->m_Value.name);

		JSONNode val(JSON_NODE);
		val.set_name("value");
		val.push_back(JSONNode("can_rx", this->m_Value.value.can_rx));
		val.push_back(JSONNode("nominal_bitrate", this->m_Value.value.nominal_bitrate));
		val.push_back(JSONNode("fast_bitrate", this->m_Value.value.fast_bitrate));
		val.push_back(JSONNode("sample_point", this->m_Value.value.sample_point));
		val.push_back(JSONNode("acq_speed", this->m_Value.value.acq_speed));
		val.push_back(JSONNode("frame_limit", this->m_Value.value.frame_limit));
		val.push_back(JSONNode("invert_bit", this->m_Value.value.invert_bit));		
		n.push_back(val);
		n.push_back(JSONNode("access_mode", this->m_Value.access_mode));

		return n;
	}
};


//I2CParameters specialization of function
template <>
inline CANParameters GetValueFromJSON<CANParameters>(JSONNode _node, const char* _at)
{
	uint8_t can_rx = _node.at(_at).at("can_rx").as_int();
	uint32_t nominal_bitrate = _node.at(_at).at("nominal_bitrate").as_int();
	uint32_t fast_bitrate = _node.at(_at).at("fast_bitrate").as_int();
	float    sample_point = _node.at(_at).at("sample_point").as_float();
	uint32_t acq_speed = _node.at(_at).at("acq_speed").as_int();
	uint16_t frame_limit = _node.at(_at).at("frame_limit").as_int();
	uint8_t invert_bit = _node.at(_at).at("invert_bit").as_int();
	return {can_rx, nominal_bitrate, fast_bitrate, sample_point,frame_limit, acq_speed,invert_bit};
}
#endif

class CANDecoder : public Decoder
{
public:
    CANDecoder(const std::string& _name = "can");
    ~CANDecoder();

    void SetParameters(const CANParameters& _new_params);
    void Decode(const uint8_t* _input, uint32_t _size) override;
    bool IsParametersChanged() override;
    void UpdateParameters() override;
    void UpdateSignals() override;

    std::vector<OutputPacket> GetSignal() { return m_Result; }

// this enum for Control byte in output structure
	enum CANAnnotations
	{
		PAYLOAD_DATA 	= 0,  // 'Payload data'
		START_OF_FRAME 	= 1,  // 'Start of frame'
		END_OF_FRAME 	= 2,  // 'End of frame'
		ID 				= 3,  // 'Identifier'
		EXT_ID 			= 4,  // 'Extended identifier'
		FULL_ID 		= 5,  // 'Full identifier'
		IDE 			= 6,  // 'Identifier extension bit'
		RESERV_BIT		= 7,  // 'Reserved bit 0 and 1'
		RTR 			= 8,  // 'Remote transmission request'
		SRR				= 9,  // 'Substitute remote request'
		DLC             = 10, // 'Data length count'
		CRC_SEQ			= 11, // 'CRC sequence'
		CRC_DELIMITER   = 12, // 'CRC delimiter'
		ACK_SLOT		= 13, // 'ACK slot'
		ACK_DELIMITER	= 14, // 'ACK delimiter'
		STUFF_BIT	 	= 15, // 'Stuff bit'
		WARNING			= 16, // 'Warning unknow'
		BIT				= 17, // 'Bit'
		ERROR_1 		= 18, // 'Start of frame (SOF) must be a dominant bit'
		ERROR_2				, // 'Data length code (DLC) > 8 is not allowed'
		ERROR_3				, // 'End of frame (EOF) must be 7 recessive bits'
		WARNING_1       	, // 'Identifier bits 10..4 must not be all recessive'
		WARNING_2			, // 'CRC delimiter must be a recessive bit'
		WARNING_3			, // 'ACK delimiter must be a recessive bit'
		BRS					, // 'Bit rate switch'
		ESI 				, // 'Error state indicator'
		CRC_LEN             , // 'Crc type
		RESERV_BIT_FLEX		, // 'Flexible data'
		NOTHING				,
		SYNC             	
	};


private:
    bool m_ParametersUpdated;

	enum States
	{
		IDLE,
		CORRECT,
		GET_INFO
	};
	
	void ResetDecoder();

	bool IsLowState(bool _can_rx) const;
	bool IsFallEdgeState(bool _can_rx) const;
	bool Is_Stuff_Bit();

	void 	 Dom_edge_seen();
	bool 	 Handle_Bit(const bool _can_rx);
	bool	 Decode_Standard_Frame(const bool _can_rx,uint32_t bitnum);
	bool	 Decode_Extended_Frame(const bool _can_rx,uint32_t bitnum);
	bool     Decode_End_Frame(const bool _can_rx,uint32_t bitnum);
	uint32_t Get_sample_point(uint32_t _curbit);
	void     Set_Bit_Rate(uint32_t _bitrate);
	void 	 Set_Nominal_Bitrate();
	void 	 Set_Fast_Bitrate();
	uint32_t DLC2Len(uint8_t _dlc);
	uint32_t Convert(const std::vector<uint32_t> &_bits);
	void	 ResetState();
	std::vector<OutputPacket> PackResult(std::vector<OutputPacket> &_data);

	bool m_OldCanRX; // Old state of RX line
	
	uint32_t m_SS_block; // start smaple of ID;
	uint32_t m_SS_bit12; // RTR or SRR bit
	uint32_t m_ID;
	int32_t  m_CurBit; // Current bit of CAN frame (bit 0 == SOF)
	uint32_t m_DataLenghtCode; // dlc
	uint32_t m_DLC_start; 
	uint32_t m_Last_Databit;
	uint32_t m_RTR;
	uint32_t m_CRC;
	uint32_t m_CRC_Len;
	float    m_Bitwidth;
	uint32_t m_Samplerate;
	int32_t  m_Samplenum; // Len of sample
	int32_t  m_SamplePoint;
	int32_t  m_SampleSkip;
	uint16_t m_Frame_limit;
	bool	 m_SyncMode;
	bool     m_IsFlexibleData; // Bit 14: FDF (Flexible data format)
	//improves the decoder's reliability when the input signal's bitrate does not exactly match the nominal rate.
	uint32_t m_dom_edge_snum;
	uint32_t m_dom_edge_bcount;
	
	std::vector<uint32_t> m_Bits; // Only actual CAN frame bits (no stuff bits)
	std::vector<uint32_t> m_RawBits; // All bits, including stuff bits
	std::vector<uint32_t> m_DataBits; 
	std::vector<uint32_t> m_SyncBits; 
	FrameFormat m_Frame_Type;

//////////////////////////////////////////
	std::string m_Name;
	uint32_t m_Ss_byte;
	uint8_t m_Databyte;


	uint32_t m_OldSamplenum;
	uint32_t m_Bitcount;

	int m_Wr;
	uint32_t m_Is_repeat_start;
	uint32_t m_Pdu_start;
	uint32_t m_Pdu_bits;

	CANAnnotations m_Cmd;
	States m_State;
	States m_OldState;

	CANParameters m_Options;
	std::vector<OutputPacket> m_TempResult;
	std::vector<OutputPacket> m_Result;
	
#ifndef CLI
	CDecodedSignal m_Signal;
	CCANParameter m_Parameters;
	pthread_mutex_t  m_mutex = PTHREAD_MUTEX_INITIALIZER;
	bool m_Start = false;
#endif
};
