#include "can_decoder.h"

#include <initializer_list>
#include <string>
#include <cstdio>
#include <limits>
#include <algorithm>
#include <cassert>


void PrintLogInFile(const char *message){
    FILE *f = fopen("/tmp/debug.log", "a+");
  	fprintf(f, "%s", message);
    fclose(f);
}

CANDecoder::CANDecoder(const std::string& _name):
	m_Name(_name),
	m_ParametersUpdated(false)
#ifndef CLI
	,
	m_Signal(m_Name + "_signal", 1, {0,0,0}),
	m_Parameters(m_Name + "_parameters", CBaseParameter::RW, m_Options, 0)
#endif
{
	ResetDecoder();
	m_Options.can_rx = 0;
	m_Options.nominal_bitrate = 1000000;
	m_Options.fast_bitrate = 2000000;
	m_Options.sample_point = 87.5;
	m_Options.invert_bit = 0;
	m_Options.frame_limit = 500;
}

void CANDecoder::SetParameters(const CANParameters& _new_params)
{
	m_Options = _new_params;
	--m_Options.can_rx; // Corrent LA line from 1 -> 0 in buffer
	m_Samplerate  = m_Options.acq_speed;
	m_Bitwidth    = (float)m_Samplerate / (float)(m_Options.nominal_bitrate);
	m_SamplePoint = (m_Bitwidth / 100.0) * m_Options.sample_point;
	fprintf(stderr,"m_Bitwidth %f, m_SamplePoint %d m_Samplerate %d\n",m_Bitwidth,m_SamplePoint,m_Samplerate);
}

bool CANDecoder::IsParametersChanged()
{
#ifndef CLI
	if(m_Parameters.IsNewValue()){
	}
	return (m_Parameters.IsNewValue()) || m_ParametersUpdated;
#else
    return false;
#endif // CLI
}

void CANDecoder::UpdateParameters()
{
#ifndef CLI
	m_Parameters.Update();
	SetParameters(m_Parameters.Value());
#endif
	m_ParametersUpdated = true;
}

void CANDecoder::UpdateSignals()
{
#ifndef CLI
	m_Signal.Update();
#endif
}

void CANDecoder::ResetDecoder()
{
	m_OldCanRX = 1;
	m_dom_edge_snum = 0;
	m_dom_edge_bcount = 0;
	m_Samplenum = 0;
	m_SampleSkip = 0;
	m_State = IDLE;
	m_SyncMode = true;
	m_TempResult.clear();
	m_SyncBits.clear();
	ResetState();
}

void CANDecoder::ResetState(){
	m_IsFlexibleData = false;
	m_Frame_Type = None;
	m_DataLenghtCode = 0;
	m_Last_Databit = 999; // Positive value that bitnum+x will never match
	m_SS_block = 0;
	m_DLC_start = 0;
	m_CRC_Len = 0;
	m_CRC = 0;
	m_DataBits.clear();
	m_RawBits.clear();
	m_Bits.clear();
	m_CurBit = 0;
	m_ID = 0;
	m_RTR = 0;
}

void CANDecoder::Decode(const uint8_t* _input, uint32_t _size)
{
#ifndef CLI
	pthread_mutex_lock(&m_mutex);
#endif
	assert(_input && "null ptr");
	fprintf(stderr,"Input data size %d\n",_size);
	assert(_size > 0 && "input vector size == 0");
	assert(_size % 2 == 0 && "odd size");
#ifndef CLI
	if (m_Parameters.IsNewValue())
	{
		m_Parameters.Update();
		SetParameters(m_Parameters.Value());
		m_Start = true;
	}


#endif
	ResetDecoder();
	m_ParametersUpdated = false;
	bool disableSynOnStart = true;
	for (size_t i = 0; i < _size; i += 2)
	{
		const uint8_t count = _input[i]; // 0 = 1 count, 1 = 2, etc.
		const uint8_t data  = _input[i + 1];
		for (uint16_t j = 0; j < count + 1; ++j)
		{

			// Get bit state of line in LA
			bool can_rx = (data & 1 << m_Options.can_rx) >> m_Options.can_rx;
			if (m_Options.invert_bit != 0){
				can_rx = !can_rx;
			}
			// char s[20];
			// sprintf(s,"%d",can_rx);
			// PrintLogInFile(s);

			if (m_State == IDLE)
			{
				if (IsLowState(can_rx)){
					Dom_edge_seen();
					m_State = CORRECT;
					disableSynOnStart = false;
				}else{
					m_TempResult.push_back(OutputPacket{NOTHING, can_rx, 1});
					if (m_Bitwidth * 8 < m_Samplenum  && disableSynOnStart) {
						m_SyncMode = false;
						disableSynOnStart = false;
					}
				}
			}
			if (m_State == CORRECT){
				int32_t pos = Get_sample_point(m_CurBit);
				m_SampleSkip = pos - m_Samplenum;
				m_State = GET_INFO;
				//printf("S %d :  %d : %d : %d\n",m_Samplenum,m_SampleSkip,m_CurBit,pos);
			}

			if (m_State == GET_INFO){
				if (IsFallEdgeState(can_rx)){
					Dom_edge_seen();
					m_State = CORRECT;
				}

				if(m_SampleSkip <= 0){
					if (m_SyncMode){
						m_SyncBits.push_back(can_rx);
						m_CurBit += 1;
						m_TempResult.push_back(OutputPacket{SYNC, can_rx, 1});
						if (m_SyncBits.size() >= 7){
							std::vector<uint32_t> last_7_bits(m_SyncBits.end() - 7, m_SyncBits.end());
							std::vector<uint32_t> data { 1, 1, 1, 1, 1, 1, 1 };
							if (last_7_bits == data){
								m_State = IDLE;
								m_SyncMode = false;

							}else{
								m_State = CORRECT;
							}
						}else{
							m_State = CORRECT;
						}
					}else{
						if (Handle_Bit(can_rx)) {
							if (m_TempResult.back().control == ERROR_3){
								m_SyncMode = true;
								m_SyncBits.clear();
								m_State = CORRECT;
							}else{
								m_State = IDLE;
							}
						}else{
							m_State = CORRECT;
						}
					}
				}else{
					m_TempResult.push_back(OutputPacket{NOTHING, can_rx, 1});
				}

				if (m_SampleSkip > 0){
					m_SampleSkip--;
				}
			}

			m_OldCanRX = can_rx;

			assert(m_Samplenum < std::numeric_limits<decltype(m_Samplenum)>::max() && "m_Samplenum overflow");
			++m_Samplenum;

		}
	}
	m_TempResult = PackResult(m_TempResult);
	int sum = 0;
	for(auto i = 0u; i < m_TempResult.size(); i ++) {
		if (sum <= m_Options.frame_limit) {
				m_Result.push_back(m_TempResult[i]);
			}
		if (m_TempResult[i].control == END_OF_FRAME) {
			sum++;
		}
	}
	fprintf(stderr,"Pack result: %d frames found: %d\n",m_TempResult.size(),sum);

#ifndef CLI
	m_Signal.Set(std::move(m_Result));
	pthread_mutex_unlock(&m_mutex);
#endif
}

std::vector<OutputPacket> CANDecoder::PackResult(std::vector<OutputPacket> &_data){
	std::vector<OutputPacket> new_result;
	for (size_t i = 0; i < _data.size(); i++){
		if (new_result.size () > 0 ){
			if ((new_result.back().control == _data[i].control) &&
					(new_result.back().data == _data[i].data)){
						uint32_t sum_size = _data[i].length + new_result.back().length;
						if (sum_size > 0xFFFF){
							new_result.back().length = 0xFFFF;
							new_result.push_back(_data[i]);
							new_result.back().length = sum_size - 0xFFFF;
						}else{
							new_result.back().length = sum_size;
						}
					}else{
						new_result.push_back(_data[i]);
					}
		}else{
			new_result.push_back(_data[i]);
		}
	}
	return new_result;
}


void CANDecoder::Dom_edge_seen(){
	m_dom_edge_snum = m_Samplenum;
	m_dom_edge_bcount = m_CurBit;
}

uint32_t CANDecoder::Get_sample_point(uint32_t _curbit){
  	uint32_t samplenum = m_dom_edge_snum;
    samplenum += m_Bitwidth * (_curbit - m_dom_edge_bcount);
    samplenum += m_SamplePoint;
    return samplenum;
}

void CANDecoder::Set_Bit_Rate(uint32_t _bitrate){
    m_Bitwidth = float(m_Samplerate) / float(_bitrate);
    m_SamplePoint = (m_Bitwidth / 100.0) * m_Options.sample_point;
}

void CANDecoder::Set_Nominal_Bitrate(){
	Set_Bit_Rate(m_Options.nominal_bitrate);
}

void CANDecoder::Set_Fast_Bitrate(){
	Set_Bit_Rate(m_Options.fast_bitrate);
}

bool CANDecoder::IsLowState(bool _can_rx) const
{
	return (_can_rx == 0);
}

bool CANDecoder::IsFallEdgeState(bool _can_rx) const
{
	return (m_OldCanRX == 1 && _can_rx == 0);
}


bool CANDecoder::Handle_Bit(const bool _can_rx){
	m_Bits.push_back(_can_rx);
	m_RawBits.push_back(_can_rx);
	uint32_t bitnum = m_Bits.size() - 1;

	if (m_IsFlexibleData && _can_rx){
		if ((bitnum == 16 && m_Frame_Type == Standart) || (bitnum == 35 && m_Frame_Type == Extended)){
			Dom_edge_seen();
			Set_Fast_Bitrate();
		}
	}

	// If this is a stuff bit, remove it from self.bits and ignore it.

	if (Is_Stuff_Bit()){
		m_TempResult.push_back(OutputPacket{STUFF_BIT, _can_rx, 1}); // Put stuff bit
		m_CurBit += 1; // Increase curbit (bitnum is not affected).
		return false;
	}else{
		m_TempResult.push_back(OutputPacket{BIT, _can_rx, 1}); // Put bit
	}


 	// Bit 0: Start of frame (SOF) bit
    if (bitnum == 0) {
        m_TempResult.push_back(OutputPacket{START_OF_FRAME, _can_rx, 1});
        if (_can_rx != 0){
			m_TempResult.push_back(OutputPacket{ERROR_1, _can_rx, 1});
		}
	}

	// Remember start of ID (see below).
	if (bitnum == 1) {
        m_SS_block = m_Samplenum;
	}

	// Bits 1-11: Identifier (ID[10..0])
	// The bits ID[10..4] must NOT be all recessive.
    if (bitnum == 11) {
		std::vector<uint32_t> id_bits(m_Bits.begin() + 1, m_Bits.end());
		m_ID = Convert(id_bits);
		m_TempResult.push_back(OutputPacket{ID, m_ID, 1});
		if ((m_ID & 0x7f0) == 0x7f0){
			m_TempResult.push_back(OutputPacket{WARNING_1, m_ID, 1});
		}
	}

	// RTR or SRR bit, depending on frame type (gets handled later).
    if (bitnum == 12){
        m_SS_bit12 = m_Samplenum;
	}

	// Bit 13: Identifier extension (IDE) bit
    // Standard frame: dominant, extended frame: recessive
    if (bitnum == 13)
	{
		m_Frame_Type = Standart;
		if (_can_rx == 1) m_Frame_Type = Extended;
		m_TempResult.push_back(OutputPacket{IDE, _can_rx, 1});
	}

	if (bitnum >= 14){
		bool ret = false;
        if (m_Frame_Type == Standart){
            ret = Decode_Standard_Frame(_can_rx, bitnum);
		}else{
            ret = Decode_Extended_Frame(_can_rx, bitnum);
		}
        // The handlers return True if a frame ended (EOF).
        if (ret) return true;
	}
	m_CurBit += 1;
	return false;
}

bool CANDecoder::Decode_Standard_Frame(const bool _can_rx,uint32_t bitnum){
	// Bit 14: FDF (Flexible data format)
    // Has to be sent dominant when FD frame, has to be sent recessive when classic CAN frame.
    if (bitnum == 14){
		m_IsFlexibleData = false;
		if (_can_rx) m_IsFlexibleData = true;
		if (m_IsFlexibleData){
			m_TempResult.push_back(OutputPacket{RESERV_BIT_FLEX, _can_rx, 1});
			m_TempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}else{
			m_TempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}
		if (m_IsFlexibleData){
			//  Bit 12: Substitute remote request (SRR) bit
			// m_Result.push_back(OutputPacket{SRR, _can_rx, 1}); // This mode default by FlexibleData
			m_DLC_start = 18;
		}else{
         	//  Bit 12: Remote transmission request (RTR) bit
            //  Data frame: dominant, remote frame: recessive
            //  Remote frames do not contain a data field.
			m_TempResult.push_back(OutputPacket{RTR, m_Bits[12], 1});
			m_DLC_start = 15;
		}
	}

	if (bitnum == 15 && m_IsFlexibleData){
        m_TempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
	}


	if (bitnum == 16 && m_IsFlexibleData){
		m_TempResult.push_back(OutputPacket{BRS, _can_rx, 1});
	}

    if (bitnum == 17 && m_IsFlexibleData){
		m_TempResult.push_back(OutputPacket{ESI, _can_rx, 1});
 	}

	// Remember start of DLC (see below).
    if (bitnum == m_DLC_start){
		m_SS_block = m_Samplenum;
	}


	// Bits 15-18: Data length code (DLC), in number of bytes (0-8).
    if (bitnum == m_DLC_start + 3){
		std::vector<uint32_t> dlc_bits(m_Bits.begin() + m_DLC_start, m_Bits.begin() + m_DLC_start + 4);
		m_DataLenghtCode = Convert(dlc_bits);
		m_TempResult.push_back(OutputPacket{DLC, m_DataLenghtCode, 1});
		m_Last_Databit = m_DLC_start + 3 + DLC2Len(m_DataLenghtCode) * 8;
		if (m_DataLenghtCode > 8 && !m_IsFlexibleData){
			m_TempResult.push_back(OutputPacket{ERROR_2, m_DataLenghtCode, 1});
		}
	}

	// Remember all databyte bits, except the very last one.
    if (bitnum >= m_DLC_start + 4 && bitnum <= m_Last_Databit){
		m_DataBits.push_back(m_Samplenum);
		if (m_DataBits.size() == 8){
			std::vector<uint32_t> data_bits(m_Bits.end() - 8, m_Bits.end());
			uint32_t value = Convert(data_bits);
			m_TempResult.push_back(OutputPacket{PAYLOAD_DATA, value, 1});
			m_DataBits.clear();
		}
	}

	if (bitnum > m_Last_Databit){
        return Decode_End_Frame(_can_rx, bitnum);
	}

	return false;
}

bool CANDecoder::Decode_Extended_Frame(const bool _can_rx,uint32_t bitnum){
	// Remember start of EID (see below).
    if (bitnum == 14){
        m_SS_block = m_Samplenum;
		m_IsFlexibleData = false;
		m_DLC_start = 35;
	}

	// Bits 14-31: Extended identifier (EID[17..0])
    if (bitnum == 31){
		std::vector<uint32_t> id_bits(m_Bits.begin() + 14, m_Bits.end());
		uint32_t eid = Convert(id_bits);
		m_TempResult.push_back(OutputPacket{EXT_ID, eid, 1});
		uint32_t fullid = m_ID << 18 | eid;
		m_TempResult.push_back(OutputPacket{FULL_ID, fullid, 1});
		// Bit 12: Substitute remote request (SRR) bit
		m_TempResult.push_back(OutputPacket{SRR, m_Bits[12], 1});
	}

	// Bit 32: Remote transmission request (RTR) bit
    // Data frame: dominant, remote frame: recessive
    // Remote frames do not contain a data field.

    // Remember start of RTR (see below).
    if (bitnum == 32) {
		m_SS_bit12 = m_Samplenum;
		m_RTR = _can_rx;
		if (!m_IsFlexibleData){
			m_TempResult.push_back(OutputPacket{RTR, _can_rx, 1});
		}
	}


	// Bit 33: RB1 (reserved bit)
    if (bitnum == 33){
		m_IsFlexibleData = false;
		if (_can_rx) m_IsFlexibleData = true;
		if (m_IsFlexibleData) {
			m_DLC_start = 37;
			m_TempResult.push_back(OutputPacket{RESERV_BIT_FLEX, _can_rx, 1});
			m_TempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}else{
			m_TempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
		}
	}


	// Bit 34: RB0 (reserved bit)
    if (bitnum == 34){
		m_TempResult.push_back(OutputPacket{RESERV_BIT, _can_rx, 1});
	}

	if (bitnum == 35 && m_IsFlexibleData){
		m_TempResult.push_back(OutputPacket{BRS, _can_rx, 1});
	}

    if (bitnum == 36 && m_IsFlexibleData){
		m_TempResult.push_back(OutputPacket{ESI, _can_rx, 1});
 	}

	// Remember start of DLC (see below).
    if (bitnum == m_DLC_start){
		m_SS_block = m_Samplenum;
	}

 	// Bits 35-38: Data length code (DLC), in number of bytes (0-8).
	 if (bitnum == m_DLC_start + 3){
		std::vector<uint32_t> dlc_bits(m_Bits.begin() + m_DLC_start, m_Bits.begin() + m_DLC_start + 4);
		m_DataLenghtCode = Convert(dlc_bits);
		m_TempResult.push_back(OutputPacket{DLC, m_DataLenghtCode, 1});
		m_Last_Databit = m_DLC_start + 3 + DLC2Len(m_DataLenghtCode) * 8;
		if (m_DataLenghtCode > 8 && !m_IsFlexibleData){
			m_TempResult.push_back(OutputPacket{ERROR_2, m_DataLenghtCode, 1});
		}
	}

	// Remember all databyte bits, except the very last one.
    if (bitnum >= m_DLC_start + 4 && bitnum <= m_Last_Databit){
		m_DataBits.push_back(m_Samplenum);
		if (m_DataBits.size() == 8){
			std::vector<uint32_t> data_bits(m_Bits.end() - 8, m_Bits.end());
			uint32_t value = Convert(data_bits);
			m_TempResult.push_back(OutputPacket{PAYLOAD_DATA, value, 1});
			m_DataBits.clear();
		}
	}

	if (bitnum > m_Last_Databit){
        return Decode_End_Frame(_can_rx, bitnum);
	}

	return false;
}

bool CANDecoder::Decode_End_Frame(const bool _can_rx,uint32_t bitnum){
 	// Remember start of CRC sequence (see below).
    if (bitnum == m_Last_Databit + 1){
		m_SS_block = m_Samplenum;
		if (m_IsFlexibleData) {
			if (DLC2Len(m_DataLenghtCode) < 16) {
				m_CRC_Len = 27; // 17 + SBC + stuff bits
			}else{
				m_CRC_Len = 32; // 21 + SBC + stuff bits
			}
		}else{
			m_CRC_Len = 15;
		}
	}


	// CRC sequence (15 bits, 17 bits or 21 bits)
	if (bitnum == m_Last_Databit + m_CRC_Len){
		if (m_IsFlexibleData) {
			if (DLC2Len(m_DataLenghtCode) < 16) {
				m_TempResult.push_back(OutputPacket{CRC_LEN, 17, 1});
			}else{
				m_TempResult.push_back(OutputPacket{CRC_LEN, 21, 1});
			}
		}else{
			m_TempResult.push_back(OutputPacket{CRC_LEN, 15, 1});
		}

		uint32_t x = m_Last_Databit + 1;
		std::vector<uint32_t> crc_bits(m_Bits.begin() + x, m_Bits.begin() + x + m_CRC_Len);
		m_CRC = Convert(crc_bits);
		m_TempResult.push_back(OutputPacket{CRC_SEQ, m_CRC, 1});
	}

    // CRC delimiter bit (recessive)
    if (bitnum == m_Last_Databit + m_CRC_Len + 1){
		m_TempResult.push_back(OutputPacket{CRC_DELIMITER, _can_rx, 1});
		if (_can_rx != 1) {
			m_TempResult.push_back(OutputPacket{WARNING_2, _can_rx, 1});
		}
		if (m_IsFlexibleData) {
			Set_Nominal_Bitrate();
		}
	}

 	// ACK slot bit (dominant: ACK, recessive: NACK)
    if (bitnum == m_Last_Databit + m_CRC_Len + 2){
		m_TempResult.push_back(OutputPacket{ACK_SLOT, _can_rx, 1});
	}

 	// ACK delimiter bit (recessive)
    if (bitnum == m_Last_Databit + m_CRC_Len + 3){
		m_TempResult.push_back(OutputPacket{ACK_DELIMITER, _can_rx, 1});
		if (_can_rx != 1) {
			m_TempResult.push_back(OutputPacket{WARNING_3, _can_rx, 1});
		}
	}

    // Remember start of EOF (see below).
    if (bitnum == m_Last_Databit + m_CRC_Len + 4){
		m_SS_block = m_Samplenum;
	}

	// End of frame (EOF), 7 recessive bits
    if (bitnum == m_Last_Databit + m_CRC_Len + 10){
		m_TempResult.push_back(OutputPacket{END_OF_FRAME, _can_rx, 1});
		std::vector<uint32_t> last_7_bits(m_RawBits.end() - 7, m_RawBits.end());
		std::vector<uint32_t> data { 1, 1, 1, 1, 1, 1, 1 };
		if (last_7_bits != data){
			m_TempResult.push_back(OutputPacket{ERROR_3, 0, 1});
		}
		ResetState();
		return true;
	}
	return false;
}

bool CANDecoder::Is_Stuff_Bit(){
	// Don't chack end of frame.
	if (m_Bits.size() > m_Last_Databit + 17)
		return false;

	std::vector<uint32_t> last_6_bits(m_RawBits.end() - 6, m_RawBits.end());
	std::vector<uint32_t> data_1 { 0, 0, 0, 0, 0, 1 };
	std::vector<uint32_t> data_2 { 1, 1, 1, 1, 1, 0 };
	if (last_6_bits != data_1 && last_6_bits != data_2){
		return false;
	}
	//  Stuff bit. Keep it in self.rawbits, but drop it from self.bits.
	m_Bits.pop_back();
	return true;
}

uint32_t CANDecoder::Convert(const std::vector<uint32_t> &_bits){
	uint32_t x = 0;
	for (uint32_t i = 0 ;i < _bits.size() ; i++)
	{
    	if (_bits[i] > 0 ) {
			x = x | (_bits[i] << (_bits.size() - i - 1));
		}
	}
	return x;
}

uint32_t CANDecoder::DLC2Len(uint8_t _dlc){
	uint32_t code_page[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
	return code_page[_dlc];
}

CANDecoder::~CANDecoder()
{
#ifndef CLI
	CDataManager* man = CDataManager::GetInstance();
	assert(man && "null ptr");
	if (man)
	{
		man->UnRegisterSignal(m_Signal.GetName());
		man->UnRegisterParam(m_Parameters.GetName());
	}
#endif
}
