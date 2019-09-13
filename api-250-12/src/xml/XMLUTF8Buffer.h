#pragma once
#include <locale>
#include <string.h>
#include "XMLBuffer.h"

namespace XML
{
	struct UTF8Char
	{
		char B1 = 0;
		char B2 = 0;
		char B3 = 0;
		char B4 = 0;
		
		char Lenght = 0;
	};

	class UTF8Buffer : public Buffer
	{	
	public:
		UTF8Buffer(char *buffer,int BufSize);
		~UTF8Buffer();
		bool     ReadBOM();
//		UTF8Char NextRead();
//		UTF8Char NextRead(int BytePosition) {};
		bool     CheckBuffer();

		// If char is ANSI to return code else return 0;
		char     ReadANSI();
		char     ReadNextANSI();
		char*    ReadANSI(int Count);
		char*    ReadNextANSI(int Count);

	};
}

