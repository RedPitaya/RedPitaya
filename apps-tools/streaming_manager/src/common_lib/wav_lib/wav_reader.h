#ifndef WAV_LIB_WAVRAEDER_H
#define WAV_LIB_WAVRAEDER_H

#include <fstream>
#include <stdint.h>

using namespace std;

class CWaveReader
{
	typedef struct WavHeader
	{
		uint8_t RIFF[4];		// RIFF Header Magic header
		uint32_t fileSize;		// File size - 8
		uint8_t WAVE[4];		// WAVE Header
		uint8_t fmt[4];			// FMT header
		uint32_t Subchunk1Size; // Size of the fmt chunk - 16
		uint16_t AudioFormat;	// Audio format 1=PCM
		uint16_t NumOfChan;		// Number of channels 1=Mono 2=Sterio
		uint32_t SamplesPerSec; // Sampling Frequency in Hz
		uint32_t bytesPerSec;	// bytes per second
		uint16_t blockAlign;	// 2=16-bit mono, 4=16-bit stereo
		uint16_t bitsPerSample; // Number of bits per sample
		/* "data" sub-chunk */
		uint8_t Subchunk2ID[4]; // "data"  string
		uint32_t Subchunk2Size; // Sampled data length
	} WavHeader_t;

public:
	CWaveReader();
	~CWaveReader();

	auto openFile(string fileName) -> bool;
	auto getHeader() -> WavHeader_t;
	auto getDataSize() -> uint64_t;
	auto getBuffers(uint8_t **ch1, size_t *size_ch1, uint8_t **ch2, size_t *size_ch2) -> bool;

private:
	std::fstream m_read_fs;
	WavHeader_t m_header;
	uint64_t m_dataSize;
};

#endif
