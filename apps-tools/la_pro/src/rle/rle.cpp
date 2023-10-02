#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdint>

using namespace std;

typedef uint8_t byte;

size_t readFile(const char* _fname, byte*& _buf)
{
	auto f = fopen(_fname, "rb");
	fseek(f, 0, SEEK_END);
	size_t flen = ftell(f);
	fseek(f, 0, SEEK_SET);

	_buf = new byte[flen];
	auto len = fread(_buf, sizeof(*_buf), flen, f);
	fclose(f);

	return len;
}

size_t writeFile(const char* _fname, const byte* _buf, size_t _size)
{
	auto f = fopen(_fname, "wb");
	size_t len = fwrite(_buf, sizeof(*_buf), _size, f);
	fclose(f);

	return len;
}

void SrToRle(const char* _in, const char* _out)
{
	byte* buf = nullptr;
	auto bufSize = readFile(_in, buf);

	if (!buf || !bufSize) {
		puts("read error");
		return;
	}

	vector<byte> result;
	int seqLen = 0;
	byte oldValue = buf[0];
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
	byte* buf = nullptr;
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

void RleToSr(const char* _in, const char* _out)
{
	byte* buf = nullptr;
	auto bufSize = readFile(_in, buf);

	if (!buf || !bufSize) {
		puts("read error");
		return;
	}

	vector<byte> result;
	for (size_t i = 0; i < bufSize; i += 2)
	{
		const size_t seqLen = buf[i] + 1;
		const byte value = buf[i + 1];
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
	if (argc == 4 && string("s2r") == argv[1])
		SrToRle(argv[2], argv[3]);
	else if (argc == 5 && string("split") == argv[1])
		Split(argv[2], argv[3],argv[4]);
	else if (argc == 4 && string("r2s") == argv[1])
		RleToSr(argv[2], argv[3]);
	else
		puts("SR to RLE converter args:\ns2r | split input_file output_file output_file2 | r2s input_file output_file");
}
