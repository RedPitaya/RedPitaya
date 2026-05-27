#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdint>

using namespace std;

size_t readFile(const char* _fname, uint8_t*& _buf)
{
	auto f = fopen(_fname, "rb");
	fseek(f, 0, SEEK_END);
	size_t flen = ftell(f);
	fseek(f, 0, SEEK_SET);

	_buf = new uint8_t[flen];
	auto len = fread(_buf, sizeof(*_buf), flen, f);
	fclose(f);

	return len;
}

size_t writeFile(const char* _fname, const uint8_t* _buf, size_t _size)
{
	auto f = fopen(_fname, "wb");
	size_t len = fwrite(_buf, sizeof(*_buf), _size, f);
	fclose(f);

	return len;
}

void Sr16ToRle8(const char* _in, const char* _out)
{
	uint8_t* buf = nullptr;
	auto bufSize = readFile(_in, buf);

	if (!buf || !bufSize) {
		puts("read error");
		return;
	}

	vector<uint8_t> result;
	int seqLen = 0;
	uint16_t* buf16 = (uint16_t*)buf;
	uint8_t oldValue = buf16[0];
	for (size_t i = 0; i < bufSize / 2; ++i)
	{
		if (oldValue == (uint8_t)buf16[i] && seqLen < 255)
		{
			++seqLen;
		}
		else
		{
			result.push_back(seqLen);
			result.push_back(oldValue);
			seqLen = 0;
		}
		oldValue = (uint8_t)buf16[i];
	}
	result.push_back(seqLen);
	result.push_back(oldValue);

	if (!writeFile(_out, result.data(), result.size()))
		puts("write error");
	delete[] buf;
}

void Sr8ToRle8(const char* _in, const char* _out)
{
	uint8_t* buf = nullptr;
	auto bufSize = readFile(_in, buf);

	if (!buf || !bufSize) {
		puts("read error");
		return;
	}

	vector<uint8_t> result;
	int seqLen = 0;
	uint8_t oldValue = buf[0];
	for (size_t i = 1; i < bufSize; ++i)
	{
		if (oldValue == buf[i] && seqLen < 255)
		{
			++seqLen;
		}
		else
		{
			result.push_back(seqLen);
			result.push_back(oldValue);
			seqLen = 0;
		}
		oldValue = buf[i];
	}
	result.push_back(seqLen);
	result.push_back(oldValue);

	if (!writeFile(_out, result.data(), result.size()))
		puts("write error");
	delete[] buf;
}

void Split(const char* _in, const char* _out, const char* _out2)
{
	uint8_t* buf = nullptr;
	auto bufSize = readFile(_in, buf);
	if (!buf || !bufSize) {
		puts("read error");
		return;
	}

	vector<uint8_t> result1;
	vector<uint8_t> result2;

	for (size_t i = 0; i < bufSize; i+=2)
	{
		result1.push_back(buf[i]);
		result2.push_back(buf[i+1]);
	}

	if (!writeFile(_out, result1.data(), result1.size()))
		puts("write error");
	if (!writeFile(_out2, result2.data(), result2.size()))
		puts("write error");
	delete[] buf;
}

void Rle8ToSr8(const char* _in, const char* _out)
{
	uint8_t* buf = nullptr;
	auto bufSize = readFile(_in, buf);

	if (!buf || !bufSize) {
		puts("read error");
		return;
	}

	vector<uint8_t> result;
	for (size_t i = 0; i < bufSize; i += 2)
	{
		const size_t seqLen = buf[i] + 1;
		const uint8_t value = buf[i + 1];
		for (size_t j = 0; j < seqLen; ++j)
		{
			result.push_back(value);
		}
	}

	if (!writeFile(_out, result.data(), result.size()))
		puts("write error");
	delete[] buf;
}

int main(int argc, char** argv)
{
	if (argc == 4 && string("s8-r8") == argv[1])
		Sr8ToRle8(argv[2], argv[3]);
	else if (argc == 4 && string("s16-r8") == argv[1])
		Sr16ToRle8(argv[2], argv[3]);
	else if (argc == 5 && string("split") == argv[1])
		Split(argv[2], argv[3], argv[4]);
	else if (argc == 4 && string("r8-s8") == argv[1])
		Rle8ToSr8(argv[2], argv[3]);
	else
		printf("RLE converter tool.\n" \
			   "\tUnpacks RLE data into a byte array.\n" \
			   "\tUsage:\n" \
			   "\t\ts8-r8 input_file output_file. Convert 8 bit samples to 8 bit RLE\n" \
			   "\t\ts16-r8 input_file output_file. Convert 16 bit samples to 8 bit RLE. The least significant bits are used.\n" \
			   "\t\tr8-s8 input_file output_file. Convert 8 bit RLE to 8 bit samples.\n" \
			   "\t\tsplit input_file output_file output_file2\n" \
			   "\t\tsplit - Splits RLE file into two equal parts\n");
}
