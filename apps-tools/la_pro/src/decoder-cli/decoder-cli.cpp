#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <getopt.h>

#include <iostream>

#include "../can_decoder.h"
#include "../i2c_decoder.h"
#include "../spi_decoder.h"
#include "../uart_decoder.h"

static int g_verbose, g_help;
static std::string g_interface, g_sda, g_scl, g_clk, g_cs, g_cspol, g_cpol, g_cpha, g_data;
static std::string g_rx="1", g_baudrate, g_samplerate, g_bitorder, g_databits, g_parity;
static std::string g_nom_bitrate = "1000000",g_fast_bitrate = "2000000",g_sample_point="70";
static bool g_inverted_rx = false;
static std::string g_stopbits = "10";

void printHelp()
{
	puts("Decoder-cli: [options] intput_file output_file\n"
		 "  -i protocol i2c | spi | uart | can\n"
		 "  --[sda | scl | clk | cs | cspol | cpol | cpha | data | line | baudrate | samplerate]\n"
		 "  -v verbose\n"
		 "  -h help\n\n"
		 "UART parametres:\n"
		 "  -r\tLine (0 - 7)\n"
		 "  -b\tBaudrate\n"
		 "  -S\tSamplerate\n"
		 "  -o\tBitorder (lsb | msb)\n"
		 "  -D\tData bits (5 - 9)\n"
		 "  -K\tStop bits (05 | 10 | 15 | 20)\n"
		 "  -p\tParity (none | odd | even)\n"
		 "  -n\tInverted logic levels (1 | 0)\n\n"
		 "CAN parametres:\n"
		 "  -r\tLine (0 - 7)\n"
		 "  -n\tNominal bitrate (default: 1Mhz)\n"
		 "  -f\tFast bitrate (default: 2Mhz)\n"
		 "  -S\tSamplerate\n"
		 "  -P\tSample point (default: 70.0%)\n"
		 
		 "Usage:"
		 "  decoder-cli -v -i uart -r 1 -b 9600 -S 625000 -o lsb -D 8 -p none -n 0 -K 10 intput_file output_file\n"
		 "  decoder-cli -v -i i2c --scl 1 --sda 2 intput_file output_file\n"
		 "  decoder-cli -v -i spi --cs 1 --clk 2 --data 3 --cspol low --cpol 0 --cpha 0 -o msb intput_file output_file\n\n"
		 "  decoder-cli -v -i can intput_file output_file\n\n"
	);
}

void printDecodedData(const std::string& _prot, const std::vector<OutputPacket>& _data)
{
	if (_prot == "i2c")
	{
		enum I2CAnnotations
		{
			START = 1,
			REPEAT_START,
			STOP,
			ACK,
			NACK,
			READ_ADDRESS,
			WRITE_ADDRESS,
			DATA_READ,
			DATA_WRITE,
			NOTHING
		};

		const char* ctrl_str[] = {
			"",
			"Start",
			"Start repeat",
			"Stop",
			"ACK",
			"NACK",
			"Address read",
			"Address write",
			"Data read",
			"Data write",
			"NOTHING"
		};

		printf("I2C:\nOUTPUT DATA SIZE = %zu\n", _data.size());
		for (const auto& d : _data)
		{

			if (d.control >= READ_ADDRESS && d.control <= DATA_WRITE)
			{
				printf("%s: %02X\n", ctrl_str[d.control], d.data);
			}
			else if(d.control != NOTHING)
			{
				printf("%s\n", ctrl_str[d.control]);
			}
			//printf("%15s: %02X %20d\n", ctrl_str[d.control], d.data, d.length);
		}
	}
	else if (_prot == "spi")
	{
		puts("SPI:");
		for (const auto& d : _data)
		{
			if (d.control == 0) // data only
				printf("%02X\n", d.data);
		}
	}
	else if(_prot == "uart")
	{
		puts("UART:");
		for(const auto& d : _data)
		{
			if(!(d.control & (1 << 4)) && !(d.control & (1 << 5)) && !(d.control & (1 << 6)) && !(d.control & (1 << 7)))
			{
				printf("%02X\n", d.data);
			}
		}
	}
	else if (_prot == "can")
	{
		const char* ctrl_str[] = {
			 "Payload data",
             "Start of frame",
             "End of frame",
             "Identifier",
             "Extended identifier",
             "Full identifier",
             "Identifier extension bit",
             "Reserved bit",
             "Remote transmission request",
             "Substitute remote request",
             "Data length count",
             "CRC sequence",
             "CRC delimiter",
             "ACK slot",
             "ACK delimiter",
             "Stuff bit",
             "Warning",
             "Bit",
			 "Start of frame (SOF) must be a dominant bit",
			 "Data length code (DLC) > 8 is not allowed",
			 "End of frame (EOF) must be 7 recessive bits",
			 "Identifier bits 10..4 must not be all recessive",
			 "CRC delimiter must be a recessive bit",
			 "ACK delimiter must be a recessive bit",
			 "Bit rate switch",
			 "Error state indicator",
			 "CRC type",
			 "Flexible data"
		};

		printf("CAN:\nOUTPUT DATA SIZE = %zu\n", _data.size());
		for (const auto& d : _data)
		{
			if (d.control != CANDecoder::CANAnnotations::NOTHING){
				switch (d.control)
				{
					case CANDecoder::CANAnnotations::PAYLOAD_DATA:
						printf("%s: 0x%02X\n", ctrl_str[d.control], d.data);
						break;
					case CANDecoder::CANAnnotations::BIT:
						printf("%s: %02X\n", ctrl_str[d.control], d.data);
						break;
					case CANDecoder::CANAnnotations::IDE:
						printf("%s: %02X (%s)\n", ctrl_str[d.control], d.data,d.data == 1? "extended":"standart");
						break;
					default:
						printf("%s: 0x%02X\n", ctrl_str[d.control], d.data);
						break;
				} 
			}
		}
	}
	else
	{
		puts("unknow interface:");
		for (const auto& d : _data)
		{
			printf("%x %x %x\n", d.control, d.data, d.length);
		}
	}
}

bool parseArgs(int argc, char* argv[])
{
	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"verbose", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'},
		{"interface", required_argument, 0, 'i'},
		{"sda", required_argument, 0, 'a'},
		{"scl", required_argument, 0, 's'},

		{"clk", required_argument, 0, 'k'},
		{"cs", required_argument, 0, 'c'},
		{"cspol", required_argument, 0, 'C'},
		{"cpol", required_argument, 0, 'l'},
		{"cpha", required_argument, 0, 'L'},
		{"data", required_argument, 0, 'd'},

		// UART parametres
		{"line", required_argument, 0, 'r'},
		{"baudrate", required_argument, 0, 'b'},
		{"samplerate", required_argument, 0, 'S'},
		{"bitorder", required_argument, 0, 'o'},
		{"databits", required_argument, 0, 'D'},
		{"parity", required_argument, 0, 'p'},
		{"negative", required_argument, 0, 'n'},
		{"stops", required_argument, 0, 'K'},
		//----------------

		// CAN parametres
		{"fast", required_argument, 0, 'f'},
		{"s_point", required_argument, 0, 'P'},
		//----------------

		{0, 0, 0, 0}
	};
	/* getopt_long stores the option index here. */
	int option_index = 0;
	int opt;

	while ((opt = getopt_long(argc, argv, "vhi:a:s:k:c:C:d:r:b:S:o:D:p:l:L:n:K:f:", long_options, &option_index)) != -1)
	{
		switch (opt)
		{
			case 'h':
			g_help = true;
			printHelp();
			break;

			case 'v':
			g_verbose = true;
			break;

			case 'i':
			g_interface = optarg;
			break;

			case 'a':
			g_sda = optarg;
			break;

			case 's':
			g_scl = optarg;
			break;

			case 'k':
			g_clk = optarg;
			break;

			case 'c':
			g_cs = optarg;
			break;

			case 'd':
			g_data = optarg;
			break;

			case 'C':
			g_cspol = optarg;
			break;

			case 'l':
			g_cpol = optarg;
			break;

			case 'L':
			g_cpha = optarg;
			break;

			case 'o':
			g_bitorder = optarg;
			break;

			// UART parametres
			case 'r':
			g_rx = optarg;
			break;

			case 'b':
			g_baudrate = optarg;
			break;

			case 'S':
			g_samplerate = optarg;
			break;

			case 'D':
			g_databits = optarg;
			break;

			case 'p':
			g_parity = optarg;
			break;

			case 'K':
			g_stopbits = optarg;
			break;

			case 'n':
			g_inverted_rx = (bool)atoi(optarg);
			g_nom_bitrate = optarg;
			break;
			//-----------------

			// CAN parametres
			case 'f':
			g_fast_bitrate = optarg;
			break;

			case 'P':
			g_sample_point = optarg;
			break;

			default:
			puts("incorrect args");
			printHelp();
		}
	}

	return true;
}

size_t readFile(const std::string& _path, uint8_t*& _data)
{
	timeval tv1, tv2;
	gettimeofday(&tv1, 0);

	FILE* in = fopen(_path.c_str(), "rb");
	fseek(in, 0, SEEK_END);
	size_t len = ftell(in);
	fseek(in, 0, SEEK_SET);

	_data = new uint8_t[len];
	len = fread(_data, sizeof(uint8_t), len, in);
	fclose(in);

	gettimeofday(&tv2, 0);
	if (g_verbose)
		printf("read file = %.3f msec\n", (tv2.tv_sec*1000. + tv2.tv_usec/1000.) - (tv1.tv_sec*1000. + tv1.tv_usec/1000.));

	return len;
}

void decode(const std::string& _prot, uint8_t* _data, size_t _size, std::vector<OutputPacket>& _result)
{
	timeval tv1, tv2;
	gettimeofday(&tv1, 0);

	if (_prot == "i2c")
	{
		I2CParameters params = {};
		params.sda = (uint8_t)atoi(g_sda.c_str());
		params.scl = (uint8_t)atoi(g_scl.c_str());

		I2CDecoder i2c_decoder;
		i2c_decoder.SetParameters(params);
		i2c_decoder.Decode(_data, _size);
		_result = i2c_decoder.GetSignal();
	}
	
	if (_prot == "can")
	{
		CANParameters params = {};
		params.can_rx = (uint8_t)atoi(g_rx.c_str());; 
		params.nominal_bitrate = (uint32_t)atoi(g_nom_bitrate.c_str());
		params.fast_bitrate = (uint32_t)atoi(g_fast_bitrate.c_str());
		params.sample_point = (float)atof(g_sample_point.c_str());
		params.acq_speed = (uint32_t)atoi(g_samplerate.c_str());
		params.invert_bit = 0;
		CANDecoder can_decoder;
		can_decoder.SetParameters(params);
		can_decoder.Decode(_data, _size);
		_result = can_decoder.GetSignal();
		
	}

	if (_prot == "spi")
	{
		SpiParameters params = {};
		params.clk = (uint8_t)atoi(g_clk.c_str());
		params.data = (uint8_t)atoi(g_data.c_str());
		params.cs = (uint8_t)atoi(g_cs.c_str());

		params.cpol = (uint8_t)atoi(g_cpol.c_str());
		params.cpha = (uint8_t)atoi(g_cpha.c_str());

		if(g_cspol == (std::string)"high")
			params.cs_polarity = ActiveHigh;
		else
			params.cs_polarity = ActiveLow;

		if(g_bitorder == (std::string)"lsb")
			params.bit_order = LsbFirst;
		else
			params.bit_order = MsbFirst;

		SpiDecoder spi_decoder;
		spi_decoder.SetParameters(params);
		spi_decoder.Decode(_data, _size);
		_result = spi_decoder.GetSignal();
	}
	if (_prot == "uart")
	{
		UARTParameters params = {};
		params.rx = (uint8_t)atoi(g_rx.c_str());
		params.baudrate = (uint32_t)atoi(g_baudrate.c_str());
		params.samplerate = (uint32_t)atoi(g_samplerate.c_str());

		if(g_bitorder == (std::string)"msb")
			params.bitOrder = MSB_FIRST;
		else
			params.bitOrder = LSB_FIRST;

		if((uint8_t)atoi(g_databits.c_str()) < 10 && (uint8_t)atoi(g_databits.c_str()) > 4)
			params.num_data_bits = (NumDataBits)atoi(g_databits.c_str());
		else
			params.num_data_bits = DATA_BITS_8;

		if(g_parity == (std::string)"odd")
			params.parity = ODD;
		else if(g_parity == (std::string)"even")
			params.parity = EVEN;
		else
			params.parity = NONE;

		params.invert_rx = g_inverted_rx;

		if(g_stopbits == "05")
			params.num_stop_bits = STOP_BITS_05;
		else if(g_stopbits == "10")
			params.num_stop_bits = STOP_BITS_10;
		else if(g_stopbits == "15")
			params.num_stop_bits = STOP_BITS_15;
		else if(g_stopbits == "20")
			params.num_stop_bits = STOP_BITS_20;

		UARTDecoder uart_decoder;
		uart_decoder.SetParameters(params);
		uart_decoder.Decode(_data, _size);
		_result = uart_decoder.GetSignal();
	}

	gettimeofday(&tv2, 0);
	if (g_verbose)
		printf("decode = %.3f msec\n", (tv2.tv_sec*1000. + tv2.tv_usec/1000.) - (tv1.tv_sec*1000. + tv1.tv_usec/1000.));
}

bool saveResult(const std::string& _path, const std::vector<OutputPacket>& _result)
{
	timeval tv1, tv2;
	gettimeofday(&tv1, 0);

	if (_result.size())
	{
		FILE* out = fopen(_path.c_str(), "wb");
		fwrite(_result.data(), sizeof(OutputPacket), _result.size(), out);
		fclose(out);
	}
	else
	{
		return false;
	}

	gettimeofday(&tv2, 0);
	if (g_verbose)
		printf("save file = %.3f msec\n", (tv2.tv_sec*1000. + tv2.tv_usec/1000.) - (tv1.tv_sec*1000. + tv1.tv_usec/1000.));

	return true;
}

int main(int argc, char* argv[])
{

	if (argc == 1 || !parseArgs(argc, argv))
	{
		printHelp();
		return -1;
	}


	uint8_t* file_data = 0; // need delete manually after allocate
	size_t len = readFile(argv[argc - 2], file_data);
	printf("file size = %zu B\n", len);


	std::vector<OutputPacket> resultData;
	decode(g_interface, file_data, len, resultData);


	if(!saveResult(argv[argc - 1], resultData))
	{
		puts("save file error");
		return -1;
	}


	if (g_verbose)
		printDecodedData(g_interface, resultData);

	delete[] file_data;
	return 0;
}
