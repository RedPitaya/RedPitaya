#pragma once
#include <locale>
#include <string.h>

namespace XML
{
	std::wstring GetWString(const char* buff, int size);

	
	bool IsNumber(char ch);
	bool 	 IsSpaceChar(char ch);
	bool     IsPunctuationChar(wchar_t ch);

	class Buffer
	{
	protected:
		char *buffer;
		int   lenght;
		int   bytePosition;		
	public:
		Buffer(char *buffer,int BufSize);
virtual ~Buffer();
	
virtual bool     ReadBOM();
virtual	bool     CheckBuffer();

//		UTF8Char NextRead();
		void     Seek(unsigned int BytePosition);
		int      GetPosition();
		void     First();
		void     End();
		void     MoveNext(int Count);
		void     MovePred(int Count);
		bool     IsEnd();		
		char*    GetSubBuffer(int Start, int Lenght);
		void     WriteToBuffer(char *Buffer, int SizeBuff);

		// If char is ANSI to return code else return 0;
virtual	char     ReadANSI();
virtual	char     ReadNextANSI();
virtual	char*    ReadANSI(int Count);
virtual	char*    ReadNextANSI(int Count);
		int      ReadNextSkipChars();
		

		int      FindSubANSIString(const char *Substr, int LenghtSubString);
		int      FindSubANSIChar(const char Subchar);

		void     TrimRight();	

	};
}

