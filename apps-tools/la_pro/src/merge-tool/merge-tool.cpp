#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>

#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

using namespace std;


// Parameters from command line
string g_UartFileName, g_I2cFileName, g_SpiFileName, g_OutFileName;
int g_UartRxLine = 0, g_UartTxLine = 0;
int g_I2cSclLine = 0, g_I2cSdaLine = 0;
int g_SpiCsLine = 0, g_SpiSckLine = 0, g_SpiMosiLine = 0, g_SpiMisoLine = 0;
bool g_PrintHelpFlag = false;
bool g_PrintSizesFlag = false;

// Line numbers
const int rx = 0;
const int tx = 1;
const int scl = 2;
const int sda = 3;
const int cs = 4;
const int sck = 5;
const int mosi = 6;
const int miso = 7;


void printHelp()
{
	puts("\n  Merge SR files tool (Pro Elit Mega Platinum Edition)\n\n"
		"  Usage:\n"
		"  merge-tool -u <uart.sr>:1:2 -i <i2c.sr>:1:2 -s <spi.sr>:1:2:3:4 -o <output file>\n\n"
		"  -u\tSpecify UART file and bits separated by ':' (filename:rx:tx)\n"
		"  -i\tSpecify I2C file and bits separated by ':' (filename:scl:sda)\n"
		"  -s\tSpecify SPI file and bits separated by ':' (filename:cs:sck:mosi:miso)\n"
		"  -o\tSpecify ouput file\n"
		"  -z\tShow sizes of logic files\n"
		"  -h\tPrint this help page\n\n");
}

map <int, string> parseParameter(const string par)
{
	map <int, string> tokensMap;
	int lastTokenInt = 0;
	int lastSimbInt = 0;

	// Get lines data from filename parameter
	for(size_t i=0; i<par.length(); i++)
	{
		if(par[i] == ':')
		{
			stringstream token;

			for(size_t num=lastSimbInt; num<i; num++)
				token << par[num];

			tokensMap[lastTokenInt] = token.str();
			lastSimbInt = i + 1;
			lastTokenInt++;
		}

		if(i == (par.length() - 1))
		{
			stringstream token;

			for(size_t num=lastSimbInt; num<=i; num++)
				token << par[num];

			tokensMap[lastTokenInt] = token.str();
			lastSimbInt = 0;
			lastTokenInt = 0;
		}
	}

	return tokensMap;
}

int unzipSrFile(const string filename)
{
	stringstream cmd;

	cmd << "unzip -o " << filename;

	return system(cmd.str().c_str());
}

int scanDir(const string path, vector <string>& content)
{
	DIR* directory;
	struct dirent* dir;

	directory = opendir(path.c_str());

	if(directory != NULL)
	{
		while((dir = readdir(directory)) != NULL)
			content.push_back(string(dir->d_name));
		closedir(directory);

		return 0;
	}

	return -1;
}

void parseCmdParameters(int argc, char* argv[])
{
	char getOptResult;

	for(int i=0; i<argc; i++)
	{
		getOptResult = getopt(argc, argv, "u:i:s:o:hz");
		
		switch(getOptResult)
		{
			case 'u':
				g_UartFileName = optarg;
				break;
			case 'i':
				g_I2cFileName = optarg;
				break;
			case 's':
				g_SpiFileName = optarg;
				break;
			case 'o':
				g_OutFileName = optarg;
				break;
			case 'h':
				g_PrintHelpFlag = true;
				break;
			case 'z':
				g_PrintSizesFlag = true;
				break;
			default:
				break;
		}
	}
}

void concatinateLogicFiles(vector <string> files, const string tmpFileName)
{
	// Find 'logic-' files within all files
	vector <string> logicFiles = vector <string>();
	for(auto it=files.begin(); it<files.end(); it++)
		if (it->find("logic-") != string::npos)
			logicFiles.push_back(*it);

	// Concatinate logic files names
	if(logicFiles.size() > 0)
	{
		fstream tmpLogicFile;
		fstream logicFile;

		tmpLogicFile.open(tmpFileName, fstream::out | fstream::binary);

		for(auto it=logicFiles.begin(); it<logicFiles.end(); it++)
		{
			logicFile.open(*it, fstream::in | fstream::binary);

			logicFile.seekg(0, fstream::end);
			size_t size = logicFile.tellg();
			logicFile.seekg(0, fstream::beg);
			char dataFromLogicFile[size];
			logicFile.read(dataFromLogicFile, size);
			tmpLogicFile.write(dataFromLogicFile, size);

			logicFile.close();

			stringstream rmrfCmd;
			rmrfCmd << "rm -f " << it->c_str() << " metadata" << " version";
			system(rmrfCmd.str().c_str());
		}

		tmpLogicFile.close();
	}
}

int getDataFromSrFile(const string filename, const string outFilename)
{
	int uartUnzipRes = unzipSrFile(filename);
	if(uartUnzipRes == 0)
	{
		vector <string> files = vector <string>();
		int result = scanDir(string("./"), files);

		if(result == 0)
		{
			concatinateLogicFiles(files, outFilename);
		}
		else
		{
			cerr << "There is no 'logic' files" << endl;
			return -1;
		}
	}
	else
	{
		cerr << "Can't unzip file: " << filename << endl;
		return -2;
	}

	return 0;
}

uint8_t getByteFromFile(fstream& file, size_t size)
{
	char byte;

	if((size_t)file.tellg() < size)
		file.read(&byte, 1);
	else
		byte = 0;

	return (uint8_t)byte;
}

uint8_t mergeBytes(uint8_t uartByte, uint8_t i2cByte, uint8_t spiByte)
{
	uint8_t result;

	result = // UART
			(((uartByte & (1 << (g_UartRxLine-1))) >> (g_UartRxLine-1)) << rx) |
			(((uartByte & (1 << (g_UartTxLine-1))) >> (g_UartTxLine-1)) << tx) |
			// I2C
			(((i2cByte & (1 << (g_I2cSclLine-1))) >> (g_I2cSclLine-1)) << scl) |
			(((i2cByte & (1 << (g_I2cSdaLine-1))) >> (g_I2cSdaLine-1)) << sda) |
			// SPI
			(((spiByte & (1 << (g_SpiCsLine-1))) >> (g_SpiCsLine-1)) << cs) |
			(((spiByte & (1 << (g_SpiSckLine-1))) >> (g_SpiSckLine-1)) << sck) |
			(((spiByte & (1 << (g_SpiMosiLine-1))) >> (g_SpiMosiLine-1)) << mosi) |
			(((spiByte & (1 << (g_SpiMisoLine-1))) >> (g_SpiMisoLine-1)) << miso);

	return result;
}

void mergeTmpLogicFile(fstream& uart, size_t uartSize,
					fstream& i2c, size_t i2cSize,
					fstream& spi, size_t spiSize,
					fstream& out, size_t outSize)
{
	uint8_t uartByte = 0;
	uint8_t i2cByte = 0;
	uint8_t spiByte = 0;
	uint8_t outByte = 0;

	for(size_t i=0; i<outSize; i++)
	{
		if(uart.is_open())
			uartByte = getByteFromFile(uart, uartSize);
		if(i2c.is_open())
			i2cByte = getByteFromFile(i2c, i2cSize);
		if(spi.is_open())
			spiByte = getByteFromFile(spi, spiSize);

		outByte = mergeBytes(uartByte, i2cByte, spiByte);

		if(out.is_open())
		{
			char byte = (char)outByte;
			out.write(&byte, 1);
		}
	}
}

int main(int argc, char* argv[])
{
	// Get command line parameters
	parseCmdParameters(argc, argv);

	// Print help if need
	if(g_PrintHelpFlag || argc == 1)
	{
		printHelp();
		return 0;
	}

	// Get lines data from parameter
	map <int, string> uartMap = parseParameter(g_UartFileName);
	map <int, string> i2cMap = parseParameter(g_I2cFileName);
	map <int, string> spiMap = parseParameter(g_SpiFileName);

	// Get logic data from input files
	if(uartMap[0] != "")
	{
		g_UartRxLine = atoi(uartMap[1].c_str());
		g_UartTxLine = atoi(uartMap[2].c_str());

		int result = getDataFromSrFile(uartMap[0], "tmp_uart_logic");
		if(result != 0)
		{
			cerr << "Error was occured while UART file handling" << endl;
			return -1;
		}
	}
	
	if(i2cMap[0] != "")
	{
		g_I2cSclLine = atoi(i2cMap[1].c_str());
		g_I2cSdaLine = atoi(i2cMap[2].c_str());

		int result = getDataFromSrFile(i2cMap[0], "tmp_i2c_logic");
		if(result != 0)
		{
			cerr << "Error was occured while I2C file handling" << endl;
			return -1;
		}
	}

	if(spiMap[0] != "")
	{
		g_SpiCsLine = atoi(spiMap[1].c_str());
		g_SpiSckLine = atoi(spiMap[2].c_str());
		g_SpiMosiLine = atoi(spiMap[3].c_str());
		g_SpiMisoLine = atoi(spiMap[4].c_str());

		int result = getDataFromSrFile(spiMap[0], "tmp_spi_logic");
		if(result != 0)
		{
			cerr << "Error was occured while SPI file handling" << endl;
			return -1;
		}
	}

	// Check if number 0 used
	if(!g_UartRxLine || !g_UartTxLine || !g_I2cSclLine || !g_I2cSdaLine || !g_SpiCsLine || !g_SpiSckLine || !g_SpiMisoLine || !g_SpiMosiLine)
	{
		cout << "Wrong pin number (must be from 1 to 8)" << endl;
		return -3;
	}

	fstream uartLogicFile;
	size_t uartFileSize = 0;
	fstream i2cLogicFile;
	size_t i2cFileSize = 0;
	fstream spiLogicFile;
	size_t spiFileSize = 0;
	fstream outLogicFile;
	size_t outFileSize = 0;

	uartLogicFile.open("tmp_uart_logic", fstream::in | fstream::binary);
	i2cLogicFile.open("tmp_i2c_logic", fstream::in | fstream::binary);
	spiLogicFile.open("tmp_spi_logic", fstream::in | fstream::binary);

	// Get sizes of files if exists
	if(uartLogicFile.is_open())
	{
		uartLogicFile.seekg(0, fstream::end);
		uartFileSize = uartLogicFile.tellg();
		uartLogicFile.seekg(0, fstream::beg);
	}

	if(i2cLogicFile.is_open())
	{
		i2cLogicFile.seekg(0, fstream::end);
		i2cFileSize = i2cLogicFile.tellg();
		i2cLogicFile.seekg(0, fstream::beg);
	}

	if(spiLogicFile.is_open())
	{
		spiLogicFile.seekg(0, fstream::end);
		spiFileSize = spiLogicFile.tellg();
		spiLogicFile.seekg(0, fstream::beg);
	}

	// Calculate output file size
	if(uartFileSize >= i2cFileSize)
	{
		if(uartFileSize >= spiFileSize)
			outFileSize = uartFileSize;
		else
			outFileSize = spiFileSize;
	}
	else
	{
		if(i2cFileSize >= spiFileSize)
			outFileSize = i2cFileSize;
		else
			outFileSize = spiFileSize;
	}

	if(g_PrintSizesFlag)
	{
		cout << endl << "UART file size: " << uartFileSize << " bytes" << endl;
		cout << "I2C file size: " << i2cFileSize << " bytes" << endl;
		cout << "SPI file size: " << spiFileSize << " bytes" << endl;
		cout << "Output file size: " << outFileSize << " bytes" << endl << endl;
	}

	outLogicFile.open(g_OutFileName, fstream::out | fstream::binary);

	mergeTmpLogicFile(uartLogicFile, uartFileSize, i2cLogicFile, i2cFileSize, spiLogicFile, spiFileSize, outLogicFile, outFileSize);

	// Closing files
	uartLogicFile.close();
	i2cLogicFile.close();
	spiLogicFile.close();
	outLogicFile.close();

	// rm -rf for tmp_files (no need more)
	system("rm -f tmp_*");

	return 0;
}
