#include "XMLBuffer.h"
#include <stdexcept>
#include <locale>
#include <codecvt>

namespace XML
{

	Buffer::Buffer(char *buffer, int BufSize) :buffer(buffer), lenght(BufSize){
		bytePosition = 0;
	}

	Buffer::~Buffer(){}

	void Buffer::First(){
		bytePosition = 0;
	}

	void Buffer::End(){
		bytePosition = lenght;
	}

	void Buffer::MoveNext(int Count){
		bytePosition += Count;
		if (bytePosition > lenght) bytePosition = lenght;
	}

	void Buffer::MovePred(int Count){
		bytePosition -= Count;
		if (bytePosition < 0) bytePosition = 0;
	}

	bool Buffer::IsEnd(){
		return (bytePosition >= lenght);
	}

	bool Buffer::ReadBOM(){
		return true;
	}

	// UTF8Char Buffer::NextRead(){
	// 	UTF8Char charu8;
	// 	while (!IsEnd()){
	// 		if ((buffer[bytePosition] & 0xF0) == 0xF0){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.B2 = buffer[bytePosition++];
	// 			charu8.B3 = buffer[bytePosition++];
	// 			charu8.B4 = buffer[bytePosition++];
	// 			charu8.Lenght = 4;
	// 			return charu8;
	// 		}

	// 		if ((buffer[bytePosition] & 0xE0) == 0xE0){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.B2 = buffer[bytePosition++];
	// 			charu8.B3 = buffer[bytePosition++];
	// 			charu8.Lenght = 3;
	// 			return charu8;
	// 		}

	// 		if ((buffer[bytePosition] & 0xC0) == 0xC0){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.B2 = buffer[bytePosition++];
	// 			charu8.Lenght = 2;
	// 			return charu8;
	// 		}

	// 		if ((buffer[bytePosition] & 0x80) == false){
	// 			charu8.B1 = buffer[bytePosition++];
	// 			charu8.Lenght = 1;
	// 			return charu8;
	// 		}
	// 		bytePosition++;
	// 	}
	// 	return charu8;
	// }

	char Buffer::ReadANSI(){
		if (!IsEnd()){
			if ((buffer[bytePosition] & 0x80) == false){
				return buffer[bytePosition];
			}
		}
		return 0;
	}

	char Buffer::ReadNextANSI(){
		if (!IsEnd()){
			if ((buffer[bytePosition] & 0x80) == false){
				return buffer[bytePosition++];
			}
		}
		return 0;
	}

	char*  Buffer::ReadANSI(int Count){
		char *buf = new char[Count];
		memset(buf, 0, Count);
		for (int i = 0; i < Count; i++){
			if ((bytePosition + i) < lenght){
				if ((buffer[bytePosition + i] & 0x80) == false){
					buf[i] = buffer[bytePosition + i];
				}
				else
					break;
			}
			else
				break;
		}
		return buf;
	}

	int Buffer::GetPosition(){
		return bytePosition;
	}

	char* Buffer::ReadNextANSI(int Count){
		char *buf = new char[Count];
		memset(buf, 0, Count);
		for (int i = 0; i < Count; i++){
			if (bytePosition < lenght){
				if ((buffer[bytePosition] & 0x80) == false){
					buf[i] = buffer[bytePosition];
				}
				else
					break;
			}
			else
				break;
			bytePosition++;
		}
		return buf;
	}

	int Buffer::FindSubANSIString(const char *Substr, int LenghtSubString){
		for (int i = bytePosition; i < lenght; i++){
			bool Find = true;
			for (int j = 0; j < LenghtSubString; j++){
				if (buffer[i + j] != Substr[j]){
					Find = false;
					break;
				}
			}
			if (Find) return i;
		}
		return -1;
	}

	int Buffer::FindSubANSIChar(const char Subchar){
		for (int i = bytePosition; i < lenght; i++){
			if (buffer[i] == Subchar)
				return i;
		}
		return -1;
	}

	void Buffer::Seek(unsigned int BytePosition){
		if (BytePosition > lenght) throw new std::runtime_error("Buffer: Position out of range");
		bytePosition = BytePosition;
	}

	bool Buffer::CheckBuffer(){
		if (lenght < 3) throw new std::runtime_error("Buffer: CheckBuffer out of range");
		return true;
	}

	char* Buffer::GetSubBuffer(int Start, int Lenght){
		if ((Start + Lenght) >= lenght) throw new std::runtime_error("Buffer: GetSubBuffer out of range");
		char *subbuf = new char[Lenght];
		for (int i = 0; i < Lenght; i++)
			subbuf[i] = buffer[Start + i];
		return subbuf;
	}

	void Buffer::WriteToBuffer(char *Buffer, int SizeBuff){
		if ((bytePosition + SizeBuff) >= lenght) throw new std::runtime_error("Buffer: WriteToBuffer out of range");
		for (int i = 0; i < SizeBuff; i++) Buffer[i] = buffer[bytePosition++];
	}

	int  Buffer::ReadNextSkipChars(){
		int skipCount = 0;

		while (IsSpaceChar(buffer[GetPosition()]) && !IsEnd()){
			MoveNext(1);
			skipCount++;
		}
		return skipCount;
	}

	bool IsSpaceChar(char ch){
		char Symbols[] = { '\t','\n','\x0b','\x0c','\r',' ' };
		for (int j = 0; j < sizeof(Symbols); j++){
			if (ch == Symbols[j])
				return true;
		}
		return false;
	}

	void   Buffer::TrimRight(){
		while (IsSpaceChar(buffer[lenght - 1]) && lenght > 0){
			lenght--;
		}
	}

	bool IsPunctuationChar(wchar_t ch){
		wchar_t Symbols[] = { 160 , 8194 ,8195 ,8201 ,173 ,150 ,8209 ,8211 ,8212 ,46 ,44 ,133 ,8230 ,58 ,59 ,33 ,451 ,63 ,64 ,42 ,35 ,145 ,8216 ,146 ,8217 ,130 ,8218 ,147 ,8220 ,148 ,8221 ,132 ,8222 ,171 ,187 ,769 ,39 ,180 ,182 ,167 ,136 ,710 ,152
		,732 ,166 ,40 ,41 ,9001 ,9002 ,8249 ,8250 ,91 ,93 ,47 ,92 ,8260 ,448 ,449 ,8254 ,175 };
		for (int j = 0; j < 59; j++){
			if (ch == Symbols[j])
				return true;
		}
		return false;
	}

	bool IsNumber(char ch){
		char Symbols[] = { '0','1','2','3','4','5','6','7','8','9' };
		for (int j = 0; j < sizeof(Symbols); j++){
			if (ch == Symbols[j])
				return true;
		}
		return false;
	}
}
