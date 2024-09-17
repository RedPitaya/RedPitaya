#include "wav_writer.h"
#include <sstream>
#include <vector>

CWaveWriter::CWaveWriter()
{
	resetHeaderInit();
	m_endianness = CWaveWriter::Endianness::LittleEndian;
}

auto CWaveWriter::resetHeaderInit() -> void
{
	m_headerInit = true;
}

auto CWaveWriter::BuildWAVStream(std::map<DataLib::EDataBuffersPackChannel, SBuffPass> new_buffs) -> std::iostream *
{
	auto ch1 = new_buffs[DataLib::CH1];
	auto ch2 = new_buffs[DataLib::CH2];
	auto ch3 = new_buffs[DataLib::CH3];
	auto ch4 = new_buffs[DataLib::CH4];

	// IF resolution = 32bit this FLOAT type data
	// Init variables

	m_numChannels = 0;
	size_t maxSamples = 0;
	uint8_t maxBitBySample = 0;
	uint32_t OSCRate = 0;
	std::vector<net_lib::net_buffer> channels;
	std::vector<uint8_t> channelsBits;
	std::vector<size_t> channelsSamples;

	if (ch1.buffer.get()) {
		m_numChannels++;
		maxSamples = maxSamples < ch1.samplesCount ? ch1.samplesCount : maxSamples;
		maxBitBySample = maxBitBySample < ch1.bitsBySample ? ch1.bitsBySample : maxBitBySample;
		OSCRate = ch1.adcSpeed;
		channels.push_back(ch1.buffer);
		channelsBits.push_back(ch1.bitsBySample);
		channelsSamples.push_back(ch1.samplesCount);
	}

	if (ch2.buffer.get()) {
		m_numChannels++;
		maxSamples = maxSamples < ch2.samplesCount ? ch2.samplesCount : maxSamples;
		maxBitBySample = maxBitBySample < ch2.bitsBySample ? ch2.bitsBySample : maxBitBySample;
		OSCRate = ch2.adcSpeed;
		channels.push_back(ch2.buffer);
		channelsBits.push_back(ch2.bitsBySample);
		channelsSamples.push_back(ch2.samplesCount);
	}

	if (ch3.buffer.get()) {
		m_numChannels++;
		maxSamples = maxSamples < ch3.samplesCount ? ch3.samplesCount : maxSamples;
		maxBitBySample = maxBitBySample < ch3.bitsBySample ? ch3.bitsBySample : maxBitBySample;
		OSCRate = ch3.adcSpeed;
		channels.push_back(ch3.buffer);
		channelsBits.push_back(ch3.bitsBySample);
		channelsSamples.push_back(ch3.samplesCount);
	}

	if (ch4.buffer.get()) {
		m_numChannels++;
		maxSamples = maxSamples < ch4.samplesCount ? ch4.samplesCount : maxSamples;
		maxBitBySample = maxBitBySample < ch4.bitsBySample ? ch4.bitsBySample : maxBitBySample;
		OSCRate = ch4.adcSpeed;
		channels.push_back(ch4.buffer);
		channelsBits.push_back(ch4.bitsBySample);
		channelsSamples.push_back(ch4.samplesCount);
	}

	m_samplesPerChannel = maxSamples;
	m_bitDepth = maxBitBySample;
	m_OSCRate = OSCRate;

	std::stringstream *memory = nullptr;
	try {
		memory = new std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	} catch (...) {
		return nullptr;
	}

	if (m_headerInit) {
		BuildHeader(memory);
		m_headerInit = false;
	}

	auto get16Bit = [&](uint8_t ch, size_t pos) -> uint16_t {
		if (channelsBits[ch] == 8) {
			if (channelsSamples[ch] > pos) {
				return (uint16_t) channels[ch].get()[pos] << 8;
			}
		}

		if (channelsBits[ch] == 16) {
			if (channelsSamples[ch] > pos) {
				return ((uint16_t *) channels[ch].get())[pos];
			}
		}
		return 0;
	};

	auto get32Bit = [&](uint8_t ch, size_t pos) -> float {
		if (channelsBits[ch] == 8) {
			if (channelsSamples[ch] > pos) {
				auto b = (int8_t *) channels[ch].get(); // keep sign
				return ((float) b[pos]) / (float) 0x7F;
			}
		}

		if (channelsBits[ch] == 16) {
			if (channelsSamples[ch] > pos) {
				auto b = (int16_t *) channels[ch].get(); // keep sign
				return ((float) b[pos]) / (float) 0x7FFF;
			}
		}

		if (channelsBits[ch] == 32) {
			if (channelsSamples[ch] > pos) {
				return ((float *) channels[ch].get())[pos];
			}
		}

		return 0;
	};

	size_t buffLen = m_numChannels * maxSamples * (maxBitBySample / 8);
	try {
		std::vector<uint8_t> cross_buff;
		cross_buff.resize(buffLen);
		for (size_t i = 0; i < maxSamples; i++) {
			for (uint8_t ch = 0; ch < m_numChannels; ch++) {
				if (m_bitDepth == 8) {
					if (channelsSamples[ch] > i) {
						cross_buff[i * m_numChannels + ch] = channels[ch].get()[i];
					} else {
						cross_buff[i * m_numChannels + ch] = 0;
					}
				}

				if (m_bitDepth == 16) {
					auto cross_buff16 = (uint16_t *) cross_buff.data();
					if (channelsSamples[ch] > i) {
						cross_buff16[i * m_numChannels + ch] = get16Bit(ch, i);
					} else {
						cross_buff16[i * m_numChannels + ch] = 0;
					}
				}

				if (m_bitDepth == 32) {
					auto cross_buff32 = (float *) cross_buff.data();
					if (channelsSamples[ch] > i) {
						cross_buff32[i * m_numChannels + ch] = get32Bit(ch, i);
					} else {
						cross_buff32[i * m_numChannels + ch] = 0;
					}
				}
			}
		}
		memory->write((const char *) cross_buff.data(), buffLen);
		if (memory->good()) {
			memory->flush();
			return memory;
		}
	} catch (std::exception &e) {
		fprintf(stderr, "[ERROR] CDataBuffer: %s\n", e.what());
	}
	delete memory;
	return nullptr;
}

auto CWaveWriter::BuildHeader(std::stringstream *memory) -> void
{
	int sampleRate = 44100;
	int32_t dataChunkSize = m_samplesPerChannel * m_numChannels * (m_bitDepth / 8);
	int16_t data_format = m_bitDepth == 32 ? 0x0003 : 0x0001;
	addStringToFileData(memory, "RIFF");

	int32_t fileSizeInBytes = 4 + 24 + 8 + dataChunkSize;
	addInt32ToFileData(memory, fileSizeInBytes);
	addStringToFileData(memory, "WAVE");

	// -----------------------------------------------------------
	// FORMAT CHUNK
	addStringToFileData(memory, "fmt ");
	addInt32ToFileData(memory, 16);						 // format chunk size (16 for PCM)
	addInt16ToFileData(memory, data_format);			 // audio format = 1
	addInt16ToFileData(memory, (int16_t) m_numChannels); // num channels
	addInt32ToFileData(memory, (int32_t) m_OSCRate);	 // sample rate

	int32_t numBytesPerSecond = (int32_t) ((m_numChannels * sampleRate * m_bitDepth) / 8);
	addInt32ToFileData(memory, numBytesPerSecond);

	int16_t numBytesPerBlock = m_numChannels * (m_bitDepth / 8);
	addInt16ToFileData(memory, numBytesPerBlock);

	addInt16ToFileData(memory, (int16_t) m_bitDepth);

	// -----------------------------------------------------------
	memory->write("data", 4);
	addInt32ToFileData(memory, dataChunkSize);
	//    std::cout << "BuildHeader: dataChunkSize " << dataChunkSize << "\n";
}

auto CWaveWriter::addStringToFileData(std::stringstream *memory, std::string s) -> void
{
	memory->write(s.data(), s.size());
}

void CWaveWriter::addInt32ToFileData(std::stringstream *memory, int32_t i)
{
	char bytes[4];

	if (m_endianness == Endianness::LittleEndian) {
		bytes[3] = (i >> 24) & 0xFF;
		bytes[2] = (i >> 16) & 0xFF;
		bytes[1] = (i >> 8) & 0xFF;
		bytes[0] = i & 0xFF;
	} else {
		bytes[0] = (i >> 24) & 0xFF;
		bytes[1] = (i >> 16) & 0xFF;
		bytes[2] = (i >> 8) & 0xFF;
		bytes[3] = i & 0xFF;
	}
	memory->write(bytes, 4);
}

void CWaveWriter::addInt16ToFileData(std::stringstream *memory, int16_t i)
{
	char bytes[2];

	if (m_endianness == Endianness::LittleEndian) {
		bytes[1] = (i >> 8) & 0xFF;
		bytes[0] = i & 0xFF;
	} else {
		bytes[0] = (i >> 8) & 0xFF;
		bytes[1] = i & 0xFF;
	}

	memory->write(bytes, 2);
}
